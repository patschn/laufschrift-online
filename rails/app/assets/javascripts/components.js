"use strict";

// ComponentMapper
// ===============

var ComponentMapper = ( function() {
		var componentInfosByCommand = {};

		var registerComponentInfo = function(info) {
			componentInfosByCommand[info.command] = info;
			return info;
		};

		var getAllComponentInfosByCommand = function() {
			return componentInfosByCommand;
		};

		var getComponentInfoForCommand = function(command) {
			if (componentInfosByCommand[command] === undefined) {
				throw new Error("Component for command '" + command + "' unknown");
			}
			return componentInfosByCommand[command];
		};

		return {
			registerComponentInfo : registerComponentInfo,
			getAllComponentInfosByCommand : getAllComponentInfosByCommand,
			getComponentInfoForCommand : getComponentInfoForCommand
		};
	}());

// Component
// =========

function Component() {
	var htmlElement = null;

	this.getAsText = function() {
		throw new Error("Pure virtual function");
	};

	// Normalerweise sollte das gleich sein, außer bei Spezialfällen wie Twitter
	this.getSignText = function() {
		return this.getAsText();
	};

	this.createInsideHTMLElement = function() {
		throw new Error("Pure virtual function");
	};

	this.hasHTMLElement = function() {
		return (htmlElement !== null);
	};

	this.getHTMLElement = function() {
		if (htmlElement !== null) {
			return htmlElement;
		}
		htmlElement = $('<div class="component"><div class="component-inner"></div></div>');
		htmlElement.data("component", this);
		this.createInsideHTMLElement(htmlElement, htmlElement.children("div.component-inner"));
		//elem.draggable();
		return htmlElement;
	};
}

function TextComponent(text) {
	Component.call(this);

	if (text === undefined) {
		text = "";
	}
	var inputElem = null;

	this.getAsText = function() {
		return SequenceCodec.escapeText(text);
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
		containerElem.addClass("component-text");
		inputElem = $('<input type="text" />');
		inputElem.val(text);
		var that = this;
		inputElem.change(function() {
			text = $(this).val();
			$(that).trigger('change');
		});
		elem.append(inputElem);
	};

	Object.defineProperties(this, {
		text : {
			get : function() {
				return text;
			},
			set : function(newtext) {
				text = newtext;
				if (inputElem) {
					inputElem.val(newtext);
				}
			},
			enumerable : true
		}
	});
}

TextComponent.prototype = new Component();

function CommandComponent(command) {
	Component.call(this);

	this.getAsText = function() {
		return "<" + command + ">";
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
		containerElem.addClass("component-command-" + this.command);
		elem.append(this.command);
	};

	Object.defineProperties(this, {
		command : {
			get : function() {
				return command;
			},
			enumerable : true
		}
	});
}

CommandComponent.prototype = new Component();

function ColorCommandComponent(command, color) {
	CommandComponent.call(this, command);

	var that = this;

	if (color === undefined) {
		color = (command === 'BGCOLOR') ? 'b' : 'y';
	}

	this.getAsText = function() {
		return "<" + this.command + " " + color + ">";
	};

	var updateClasses = function() {
		if (!that.hasHTMLElement()) {
			return;
		}
		var htmlElem = that.getHTMLElement();
		var type = (command === 'BGCOLOR') ? 'bg' : 'fg';
		htmlElem.addClass('color-' + type + '-' + color);
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
		//ColorCommandComponent.prototype.createInsideHTMLElement.apply(that, arguments);
		if (command === 'COLOR') {
			elem.append('A');
		}
		updateClasses();
	};

	Object.defineProperties(this, {
		color : {
			get : function() {
				return color;
			},
			enumerable : true
		}
	});
}

ColorCommandComponent.prototype = new CommandComponent();

function SliderCommandComponent(command, value) {
	CommandComponent.call(this, command);

	var that = this;

	if (value === undefined) {
		value = 1;
	}

	this.getAsText = function() {
		return "<" + this.command + " " + value + ">";
	};

	var updateClasses = function() {
		if (!that.hasHTMLElement()) {
			return;
		}
		var htmlElem = that.getHTMLElement();
		var type = (command === 'WAIT') ? 'WAIT' : 'SPEED';
		htmlElem.addClass(type + '-' + value);
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
		containerElem.addClass("component-command-" + this.command);
		elem.append(this.command + '&nbsp;' + value + '&nbsp;');
		updateClasses();
	};

	Object.defineProperties(this, {
		slider : {
			get : function() {
				return value;
			},
			enumerable : true
		}
	});
}

SliderCommandComponent.prototype = new CommandComponent();

function GroupComponent(components) {
    Component.call(this);
    
    if (components === undefined) {
        components = [];
    }
    if (typeof(components) === "string") {
        components = SequenceCodec.decodeFromString(components);
    }
    
    this.getAsText = function() {
        return "<GROUP " + SequenceCodec.encodeToString(components, false) + ">";
    };
    
    this.getSignText = function() {
        return SequenceCodec.encodeToString(components, true);
    };
    
	this.createInsideHTMLElement = function(containerElem, elem) {
		containerElem.addClass("component-group");
		$.each(components, function(i, component) {
		    elem.append(component.getHTMLElement());
		});
	};
    
    Object.defineProperties(this, {
        components : {
            get: function() {
                return components;
            },
            enumerable: true
        }
    });
}

GroupComponent.prototype = new Component();

// ComponentInfo
// ==============

function ComponentInfo(command, factory) {
	if (factory === undefined) {
		// Siehe https://stackoverflow.com/questions/1606797/use-of-apply-with-new-operator-is-this-possible
		// Das ruft nur den Konstruktor von CommandComponent auf, übergibt aber sowohl den gespeicherten
		// Befehl als erstes Argument als auch alle Argumente, die an factory() übergeben wurden
		factory = ( function() {
				function F(args) {
					return CommandComponent.apply(this, args);
				}


				F.prototype = CommandComponent.prototype;

				return function() {
					var args = Array.prototype.slice.call(arguments);
					args.unshift(command);
					return new F(args);
				}
			}());
	}

	Object.defineProperties(this, {
		command : {
			get : function() {
				return command;
			},
			enumerable : true
		},
		factory : {
			get : function() {
				return factory;
			},
			enumerable : true
		}
	});
}

// ToolInfo
// ========

function ToolInfo(group, componentInfo, options) {
	if (options === undefined) {
		options = {};
	}

	this.createComponent = function() {
		var extraArgs = [];
		var factoryToUse = componentInfo.factory;
		if (options.factoryArguments !== undefined) {
			extraArgs = options.factoryArguments;
		}
		if (options.overrideFactory !== undefined) {
		    factoryToUse = options.overrideFactory;
		}
		return factoryToUse.apply(componentInfo, extraArgs);
	};

	this.createToolHTMLElement = function() {
		var text = componentInfo.command;
		if (options.toolText !== undefined) {
			text = options.toolText;
		}

		var elem = $('<div class="tool">' + text + '</div>');
		elem.data("tool-info", this);
		if (componentInfo.command) {
			elem.addClass("tool-command-" + componentInfo.command);
		}
		if (options.extraClass !== undefined) {
			elem.addClass(options.extraClass);
		}
		// connectToSortable: '#', helper: 'clone' oder function()
		//elem.draggable();
		return elem;
	};

	Object.defineProperties(this, {
		group : {
			get : function() {
				return group;
			},
			enumerable : true
		},
		componentInfo : {
			get : function() {
				return componentInfo;
			},
			enumerable : true
		}
	});
}

// Registrierung der Komponenten
// =============================
var ASC333Components = {
	register : function() {
	    var textComponentInfo = new ComponentInfo(null, function(t) {
			return new TextComponent(t);
		});
		Toolbox.registerToolInfo(new ToolInfo('text', textComponentInfo, {
			toolText : 'Text'
		}));

		var openAnimations = ['LEFT', 'RIGHT', 'UP', 'DOWN', 'DOFF', 'DOBIG', 'FLASH', 'JUMP', 'OPENMID', 'OPENRIGHT', 'RAINBOW', 'RANDOM', 'SHIFTMID', 'SNOW'];
		var closeAnimations = ['CLOSEMID', 'CLOSERIGHT', 'SQUEEZEMID', 'DSNOW'];
		$.each(openAnimations, function(i, animation) {
			var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation));
			Toolbox.registerToolInfo(new ToolInfo('open_animation', info));
		});
		$.each(closeAnimations, function(i, animation) {
			var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation));
			Toolbox.registerToolInfo(new ToolInfo('close_animation', info));
		});

		var colors = ['y', 'r', 'g', 'b'];
		var colorInfo = {
			fg : ComponentMapper.registerComponentInfo(new ComponentInfo('COLOR', function(c) {
				return new ColorCommandComponent('COLOR', c);
			})),
			bg : ComponentMapper.registerComponentInfo(new ComponentInfo('BGCOLOR', function(c) {
				return new ColorCommandComponent('BGCOLOR', c);
			}))
		};
		$.each(['fg', 'bg'], function(i, type) {
			$.each(colors, function(j, color) {
				Toolbox.registerToolInfo(new ToolInfo('color', colorInfo[type], {
					extraClass : 'color-' + type + '-' + color,
					factoryArguments : [color],
					toolText : (type == 'fg') ? 'A' : '&nbsp;&nbsp;&nbsp;&nbsp;'
				}));
			});
		});

		var charwidth = ['NORMAL', 'BIG'];
		$.each(charwidth, function(i, charwidth) {
			var charwidthInfo = ComponentMapper.registerComponentInfo(new ComponentInfo(charwidth));
			Toolbox.registerToolInfo(new ToolInfo('charwidth', charwidthInfo));
		});

		Toolbox.registerToolInfo(new ToolInfo('pause', ComponentMapper.registerComponentInfo(new ComponentInfo('WAIT', function(s) {
			return new SliderCommandComponent('WAIT', s);}))
		, {
			toolText : '<div class="slider">1s<input id="wait_slider" type="range" name="points" min="1" max="9" step="1" value="1" />9s</div>',
			overrideFactory : function() { return this.factory($('#wait_slider').val()); }
		}));

		Toolbox.registerToolInfo(new ToolInfo('speed', ComponentMapper.registerComponentInfo(new ComponentInfo('SPEED', function(s) {
			return new SliderCommandComponent('SPEED', s);
		})), {
			toolText : '<div class="slider" >schnell<input id="speed_slider" type="range" name="points" min="1" max="9" step="1" value="5" />langsam</div>',
			overrideFactory : function() { return this.factory($('#speed_slider').val()); }
		}));

		var groupComponentInfo = new ComponentInfo('GROUP', function(c) { return new GroupComponent(c); });
		ComponentMapper.registerComponentInfo(groupComponentInfo);
		Toolbox.registerToolInfo(new ToolInfo('text', groupComponentInfo, {
		    toolText : 'Standardelement',
		    overrideFactory: function() {
		        var components = [
		            colorInfo.fg.factory(),
		            colorInfo.bg.factory(),
		            textComponentInfo.factory()
		        ];
		        return groupComponentInfo.factory(components);
		    }
		}));
	}
};

