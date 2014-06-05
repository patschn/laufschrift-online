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
Component.prototype.getInnerHTMLElement = function() {
    return this.getHTMLElement().children("div.component-inner");
};
Component.prototype.createInsideHTMLElement = function() {
	throw new Error("Pure virtual function");
};
Component.prototype.getAsText = function() {
    throw new Error("Pure virtual function");
};
// Normalerweise sollte das gleich sein, außer bei Spezialfällen wie Twitter
Component.prototype.getSignText = function() {
	return this.getAsText();
};


function TextComponent(text) {
	Component.call(this);

	if (text === undefined) {
		text = "";
	}
	var inputElem = null;

	this.createInsideHTMLElement = function(containerElem, elem) {
		containerElem.addClass("component-text");
//		containerElem.addClass("component-handle");
		inputElem = $('<input type="text" />');
		inputElem.val(text);
		var that = this;
		inputElem.change($.proxy(function() {
			text = inputElem.val();
			$(this).trigger('change');
		}, this));
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

TextComponent.prototype = Object.create(Component.prototype);
TextComponent.prototype.constructor = TextComponent;
TextComponent.prototype.getAsText = function() {
	return SequenceCodec.escapeText(this.text);
};


function CommandComponent(command, hasPopover) {
	Component.call(this);
	
	var that = this;
	var ci;
	try {
    	ci = ComponentMapper.getComponentInfoForCommand(command);
    } catch (e) {}
    
    var updateClasses = function() {
        if (!that.hasHTMLElement()) {
            return;
        }
        that.getHTMLElement().addClass("component-command-" + command);
    };

	this.getPopoverContents = function() {
	    var group = ci.options.exchangeableInGroup;
	    if (group === undefined) {
	        return [];
	    }
	    var tools = Toolbox.getToolInfosForGroup(group);
	    return $.map(tools, function(tool) {
	        var button = $('<div class="button-component"></div>');
	        var command = tool.componentInfo.command;
	        button.addClass('button-command-' + command);
      	    button.click(function() {
	            that.command = command;
	        });
	        return button;
	    });
	};
	
	// Damit man das Popover noch nachträglich aktivieren kann
	this.activatePopover = function() {
	    if (!hasPopover) {
	        hasPopover = true;
	    }
    	var containerElem = that.getHTMLElement();
    	if (containerElem.data('popover') ) {
    	    return false;
    	}
    	var contents = that.getPopoverContents();
    	if (contents === []) {
    	    return false;
    	}
	    var popover = $('<div></div>');
	    popover.append(contents);
    	popover.addClass('popover');
    	containerElem.append(popover);
	    containerElem.popover({ my: 'center top', at: 'center bottom', popover: popover });
	    return true;
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
	    updateClasses();
		elem.addClass('button-component');
//		containerElem.addClass("component-handle");
		if (command === 'BIG' || command === 'NORMAL') {
		    //elem.append('A');
		    //elem.addClass('button-component');
		} else {
    		elem.append(that.command);
        }
        if (hasPopover) {
            activatePopover();
		}
	};

	Object.defineProperties(this, {
		command : {
			get : function() {
				return command;
			},
			set : function(newCommand) {
			    if (that.hasHTMLElement()) {
    			    that.getHTMLElement().removeClass("component-command-" + command);
    			}
	   		    command = newCommand;
			    updateClasses();
			    $(that).trigger('change');
			},
			enumerable : true
		}
	});
}

CommandComponent.prototype = Object.create(Component.prototype);
CommandComponent.prototype.constructor = CommandComponent;
CommandComponent.prototype.getAsText = function() {
	return "<" + this.command + ">";
};

function ColorCommandComponent(command, color, hasPopover) {
	CommandComponent.call(this, command, hasPopover);

	var colorType = (command === 'BGCOLOR') ? 'bg' : 'fg';
	var parentMethods = { createInsideHTMLElement: this.createInsideHTMLElement };
	var that = this;

	if (color === undefined) {
		color = (colorType === 'bg') ? 'b' : 'y';
	}

	var updateClasses = function() {
		if (!that.hasHTMLElement()) {
			return;
		}
		var htmlElem = that.getInnerHTMLElement();
		htmlElem.addClass('button-component');
		htmlElem.addClass('button-color-' + colorType + '-' + color);
	};
	
	this.getPopoverContents = function() {
	    var colors = (colorType === 'fg') ? Config.fgColors : Config.bgColors;
    	return $.map(colors, function(color) {
	        var button = $('<div class="button-component"></div>');
	        button.addClass('button-color-' + colorType + '-' + color);
      	    button.click(function() {
	            that.color = color;
	        });
	        return button;
	    });
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
		parentMethods.createInsideHTMLElement.apply(that, arguments);
		elem.empty();
//		containerElem.removeClass("component-handle");
//		elem.addClass("component-handle");
		updateClasses();
	};

	Object.defineProperties(this, {
		color : {
			get : function() {
				return color;
			},
			set : function(newColor) {
			    if (that.hasHTMLElement()) {
    			    that.getInnerHTMLElement().removeClass('button-color-' + colorType + '-' + color);
    			}
			    color = newColor;
			    updateClasses();
			    $(that).trigger('change');
			},
			enumerable : true
		}
	});
}

ColorCommandComponent.prototype = Object.create(CommandComponent.prototype);
ColorCommandComponent.prototype.constructor = ColorCommandComponent;
ColorCommandComponent.prototype.getAsText = function() {
	return "<" + this.command + " " + this.color + ">";
};

function SliderCommandComponent(command, value) {
	CommandComponent.call(this, command);

	var that = this;

	if (value === undefined) {
		value = 1;
	}

	var updateClasses = function() {
		if (!that.hasHTMLElement()) {
			return;
		}
		var htmlElem = that.getHTMLElement();
		var type = (command === 'WAIT') ? 'WAIT' : 'SPEED';
		htmlElem.addClass(type + '-' + value);
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
		containerElem.addClass("component-command-" + that.command);
//		containerElem.addClass("component-handle");
		elem.append(that.command + '&nbsp;' + value + '&nbsp;');
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

SliderCommandComponent.prototype = Object.create(CommandComponent.prototype);
SliderCommandComponent.prototype.constructor = SliderCommandComponent;
SliderCommandComponent.prototype.getAsText = function() {
	return "<" + this.command + " " + this.slider + ">";
};


function GroupComponent(components) {
    Component.call(this);
    
    var that = this;
    
    if (components === undefined) {
        components = [];
    }
    if (typeof(components) === "string") {
        components = SequenceCodec.decodeFromString(components);
    }
    
    this.createInsideHTMLElement = function(containerElem, elem) {
        containerElem.addClass("component-group");
//		containerElem.addClass("component-handle");
        $.each(components, function(i, component) {
            elem.append(component.getHTMLElement());
            if (component instanceof CommandComponent) {
                component.activatePopover();
            }
            // Änderungen weitergeben
            $(component).on('change', function() {
                $(that).trigger('change');
            });
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

GroupComponent.prototype = Object.create(Component.prototype);
GroupComponent.prototype.constructor = GroupComponent;
GroupComponent.prototype.getAsText = function() {
    return "<GROUP " + SequenceCodec.encodeToString(this.components, false) + ">";
};
GroupComponent.prototype.getSignText = function() {
    return SequenceCodec.encodeToString(this.components, true);
};

// ComponentInfo
// ==============

function ComponentInfo(command, factory, options) {
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
		},
		options : {
		    get : function() {
		        return options;
		    },
		    enumerable: true
		},
	});
}

// ToolInfo
// ========

function ToolInfo(group, componentInfo, options) {
	if (options === undefined) {
		options = {};
	}

	options = $.extend({}, {
	    toolType: 'button'
	}, options);

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
		
		var elem = $('<div class="tool"/>');
		elem.data("tool-info", this);
		if (componentInfo.command) {
			elem.addClass("tool-command-" + componentInfo.command);
		}
		if (options.tooltip !== undefined) {
		    var tooltip = $('<div class="tooltip"/>');
		    tooltip.append(options.tooltip);
		    elem.append(tooltip);
		}
		
		if (options.toolType === 'slider') {
		    var sliderElem = $('<input type="range" />');
		    sliderElem.attr("min", options.sliderRange[0]);
		    sliderElem.attr("max", options.sliderRange[1]);
		    sliderElem.attr("step", options.sliderStep);
		    sliderElem.attr("value", options.sliderInitialValue);
		    sliderElem.attr("id", options.sliderID);
		    var sliderContainer = $('<div class="tool-slider"/>');
		    sliderContainer.append($('<span class="tool-slider-lower-limit"/>').append(options.sliderLabels[0]));
		    sliderContainer.append(sliderElem);
		    sliderContainer.append($('<span class="tool-slider-upper-limit"/>').append(options.sliderLabels[1]));
		    var sliderButton = $('<div class="button-toolbox"/>');
		    sliderButton.append(options.sliderInitialValue);
		    sliderElem.on("change", function() {
		       sliderButton.text($(this).val());
		    });
		    sliderContainer.append(sliderButton);
		    elem.append(sliderContainer);
        } else {
		    elem.append(text);

		    if (options.toolType === "button") {
        		elem.addClass("button-toolbox");
        	}
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
			var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation, undefined, { exchangeableInGroup: 'open_animation' }));
			Toolbox.registerToolInfo(new ToolInfo('open_animation', info,{
				extraClass: 'open_animation-' + animation,
				tooltip: 'Test',
				//toolText: '<span class="tooltip-'+animation+'">&nbsp;&nbsp;&nbsp;</span>'
			}));
		});
		$.each(closeAnimations, function(i, animation) {
			var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation));
			Toolbox.registerToolInfo(new ToolInfo('close_animation', info, {
				extraClass: 'close_animation-' + animation,
				toolText: '<span class="tooltip-'+animation+'">&nbsp;&nbsp;&nbsp;</span>'
			}));
		});

		var colors = { fg: Config.fgColors, bg: Config.bgColors };
		var colorInfo = {
			fg : ComponentMapper.registerComponentInfo(new ComponentInfo('COLOR', function(c,p) {
				return new ColorCommandComponent('COLOR', c, p);
			})),
			bg : ComponentMapper.registerComponentInfo(new ComponentInfo('BGCOLOR', function(c,p) {
				return new ColorCommandComponent('BGCOLOR', c, p);
			}))
		};
		$.each(['fg', 'bg'], function(i, type) {
			$.each(colors[type], function(j, color) {
				Toolbox.registerToolInfo(new ToolInfo('color', colorInfo[type], {
					extraClass : 'button-color-' + type + '-' + color,
					factoryArguments : [color],
					toolText : ''
				}));
			});
		});

		var charwidths = ['NORMAL', 'BIG'];
		$.each(charwidths, function(i, charwidth) {
			var charwidthInfo = ComponentMapper.registerComponentInfo(new ComponentInfo(charwidth));
			Toolbox.registerToolInfo(new ToolInfo('charwidth', charwidthInfo, { toolText: '' }));
		});

		Toolbox.registerToolInfo(new ToolInfo('pause', ComponentMapper.registerComponentInfo(new ComponentInfo('WAIT', function(s) {
			return new SliderCommandComponent('WAIT', s);}))
		, {
		    sliderID: 'wait_slider',
		    toolType: 'slider',
		    sliderStep: 1,
		    sliderRange: [1, 9],
		    sliderInitialValue: 5,
		    sliderLabels: ['1s', '9s'],
			overrideFactory : function() { return this.factory($('#wait_slider').val()); }
		}));

		Toolbox.registerToolInfo(new ToolInfo('speed', ComponentMapper.registerComponentInfo(new ComponentInfo('SPEED', function(s) {
			return new SliderCommandComponent('SPEED', s);
		})), {
			sliderID: 'speed_slider',
			toolType: 'slider',
			sliderStep: 1,
			sliderRange: [1, 9],
			sliderInitialValue: 5,
			sliderLabels: ['schnell', 'langsam'],
			overrideFactory : function() { return this.factory($('#speed_slider').val()); }
		}));

		var groupComponentInfo = new ComponentInfo('GROUP', function(c) { return new GroupComponent(c); });
		ComponentMapper.registerComponentInfo(groupComponentInfo);
		Toolbox.registerToolInfo(new ToolInfo('text', groupComponentInfo, {
		    toolText : 'Standardelement',
		    overrideFactory: function() {
		        var components = [
		            colorInfo.fg.factory(undefined),
		            colorInfo.bg.factory(undefined),
		            ComponentMapper.getComponentInfoForCommand('LEFT').factory(),
		            textComponentInfo.factory()
		        ];
		        return groupComponentInfo.factory(components);
		    }
		}));
		
		var clocks = ['CLOCK12', 'CLOCK24'];
		$.each(clocks, function(i, clock) {
			var clockInfo = ComponentMapper.registerComponentInfo(new ComponentInfo(clock));
			Toolbox.registerToolInfo(new ToolInfo('clock', clockInfo, { extraClass: 'clock-'+clock, toolText: '' }));
		});
	}
};

