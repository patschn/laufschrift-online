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

function Component(hasPopover) {
	var htmlElement = null;
	var popoverElem = null;
	var that = this;
	
	if (hasPopover === undefined) {
	    hasPopover = true;
	}

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
		if (hasPopover) {
		    that.activatePopover();
		}
		//elem.draggable();
		return htmlElement;
	};
	
	this.getPopoverElement = function() {
	    return popoverElem;
	};
		
	// Damit man das Popover noch nachträglich aktivieren kann
	this.activatePopover = function() {
	    if (popoverElem !== null) {
	        return false;
	    }
	    if (!that.getPopoverContents) {
	        return false;
	    }
	    if (!hasPopover) {
	        hasPopover = true;
	    }
    	var containerElem = that.getHTMLElement();
    	var elem = that.getInnerHTMLElement();
    	if (elem.data('popover') ) {
    	    return false;
    	}
    	var contents = that.getPopoverContents();
    	if (contents === undefined || contents.length === 0) {
    	    return false;
    	}
        popoverElem = $('<div></div>');
	    popoverElem.append(contents);
    	popoverElem.addClass('popover');
        //elem.append(popover);
        $('body').append(popoverElem);
        elem.addClass('component-with-popover');
	    elem.popover({ my: 'center top', at: 'center bottom', popover: popoverElem, collision: 'flipfit flipfit' });
	    return true;
	};
	
	this.destroy = function() {
	    if (popoverElem !== null) {
	        popoverElem.remove();
	    }
	    if (htmlElement !== null) {
	        htmlElement.remove();
	    }
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
// Normalerweise sollte das gleich getAsText() sein, außer bei Spezialfällen wie Twitter
Component.prototype.getSignText = function() {
	return this.getAsText();
};
Component.prototype.getPopover = function() {
    if (!this.hasHTMLElement()) {
        return null;
    }
    return this.getInnerHTMLElement().data('popover');
};


function TextComponent(text) {
	Component.call(this, false);

	if (text === undefined) {
		text = "";
	}
	text = SequenceCodec.unescapeText(text);
	var inputElem = null;
	var disableOnCreate = false;

	this.createInsideHTMLElement = function(containerElem, elem) {
		containerElem.addClass("component-text");
		inputElem = $('<input type="text" />');
		inputElem.val(text);
		var that = this;
		inputElem.change($.proxy(function() {
			text = inputElem.val();
			$(this).trigger('change');
		}, this));
		elem.append(inputElem);
		if (disableOnCreate) {
		    this.disable();
		}
	};
	
	this.caret = function() {
	    // Weitergabe an die jQuery-Funktion
	    return $.fn.caret.apply(inputElem, arguments);	
	};
	
	this.disable = function() {
	    if (inputElem) {
	        inputElem.prop("disabled", true);
	    } else {
    	    disableOnCreate = true;
    	}
	};
	
	// Damit man das Element ziehen kann wenn es deaktiviert ist,
	// der Browser erlaubt das bei dem Textfeld sonst nicht
	this.addDivCover = function() {
	    if (inputElem) {
            var dragDiv = $('<div>');
            dragDiv.css('position', 'absolute');
            var border = parseInt(inputElem.css('border-left-width')) + parseInt(inputElem.css('border-image-width')) * 2;
            dragDiv.height(inputElem.height() + border);
            dragDiv.width(inputElem.width() + border);
            this.getInnerHTMLElement().append(dragDiv);
            dragDiv.position({my: 'center', at: 'center', of: inputElem});	
        }
	};
	
	this.enable = function() {
	    if (inputElem) {
    	    inputElem.prop("disabled", false);
    	} else {
    	    disableOnCreate = false;
    	}
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
    			$(this).trigger('change');
		    },
			enumerable : true
		}
	});
}

TextComponent.prototype = Object.create(Component.prototype);
TextComponent.prototype.constructor = TextComponent;
TextComponent.prototype.getAsText = function() {
    // Muss extra codiert werden, damit leere Textelemente nicht rausfallen
	return "<TEXT " + this.getSignText() + ">";
};
TextComponent.prototype.getSignText = function() {
	return SequenceCodec.escapeText(this.text);
};
TextComponent.prototype.insertText = function(position, text) {
    this.text = this.text.substr(0, position) + text + this.text.substr(position);
};
TextComponent.prototype.insertTextAtCursor = function(text) {
    var cPos = this.caret();
    this.insertText(cPos, text);
    this.caret(cPos + text.length);
};


function CommandComponent(command, hasPopover) {
	Component.call(this, hasPopover);
	
	var parentMethods = { activatePopover: this.activatePopover };
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
        that.getInnerHTMLElement().addClass("button-command-" + command);
    };

	this.getPopoverContents = function() {
	    if (!ci) {
	        return [];
	    }
	    var group = ci.options.exchangeableInGroup;
	    if (group === undefined) {
	        return [];
	    }
	    var tools = Toolbox.getToolInfosForGroup(group);
	    return $.map(tools, function(tool) {
	        var button = tool.createToolHTMLElement();
	        var command = tool.componentInfo.command;
      	    button.click(function() {
	            that.command = command;
	            that.getPopover().hide();
	        });
	        return button;
	    });
	};
	
	this.activatePopover = function() {
	    if (!parentMethods.activatePopover.apply(that, arguments)) {
	        return false;
	    }
	    that.getPopoverElement().addClass('popover-command-' + command);
	    var group;
	    if (ci) {
	        group = ci.options.exchangeableInGroup;
	        if (group !== undefined) {
	            that.getPopoverElement().addClass('popover-group-' + group);
	        }	    
	    }
	    return true;
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
	    updateClasses();
		elem.addClass('button-component');
	};

	Object.defineProperties(this, {
		command : {
			get : function() {
				return command;
			},
			set : function(newCommand) {
			    if (that.hasHTMLElement()) {
    			    that.getHTMLElement().removeClass("component-command-" + command);
    			    that.getInnerHTMLElement().removeClass("button-command-" + command);
    			}
    			var popoverElem = that.getPopoverElement();
    			if (popoverElem !== null) {
    			    popoverElem.removeClass("popover-command-" + command);
    			    popoverElem.addClass("popover-command-" + newCommand);
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
	        var button = $('<div class="button-toolbox"></div>');
	        button.addClass('button-color-' + colorType + '-' + color);
      	    button.click(function() {
	            that.color = color;
	            that.getPopover().hide();
	        });
	        return button;
	    });
	};

	this.createInsideHTMLElement = function(containerElem, elem) {
		parentMethods.createInsideHTMLElement.apply(that, arguments);
		elem.empty();
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

function SliderCommandComponent(command, value, hasPopover) {
	CommandComponent.call(this, command, hasPopover);

	var parentMethods = { createInsideHTMLElement: this.createInsideHTMLElement };
	var that = this;
	var textContent;

	if (value === undefined) {
		value = 1;
	}
	
	this.getPopoverContents = function() {
        var i;
        var buttons = [];
        var button;
        // Zur Entkopplung der Closure von der Schleife
        function clickCB(j) {
            return function() { that.value = j; that.getPopover().hide(); };
        }
        
        for (i = 0; i <= 9; i++) {
            if (i == 0 && that.command === 'SPEED') {
                continue;
            }
            button = $('<div class="button-toolbox"></div>');
            button.addClass('button-command-' + this.command);
            button.append(i);
      	    button.click(clickCB(i));
            buttons.push(button);
        }
        return buttons;
    };

	this.createInsideHTMLElement = function(containerElem, elem) {
		parentMethods.createInsideHTMLElement.apply(that, arguments);
        elem.empty();
        textContent = $(document.createTextNode(value));
        elem.append(textContent);
	};

	Object.defineProperties(this, {
		value : {
			get : function() {
				return value;
			},
			set : function(newValue) {
                value = newValue;
			    if (that.hasHTMLElement()) {
			        textContent = $(document.createTextNode(value)).replaceAll(textContent);
			    }
			    $(that).trigger('change');
			},
			enumerable : true
		}
	});
}

SliderCommandComponent.prototype = Object.create(CommandComponent.prototype);
SliderCommandComponent.prototype.constructor = SliderCommandComponent;
SliderCommandComponent.prototype.getAsText = function() {
	return "<" + this.command + " " + this.value + ">";
};
SliderCommandComponent.prototype.getSignText = function() {
    var adjValue = this.value;
    if (this.command === 'SPEED') {
        // Skala invertieren für SPEED, damit es intuitiver ist
        // Nur für die Laufschrift!
        adjValue = (10 - adjValue);
    }
	return "<" + this.command + " " + adjValue + ">";
};

function LinebreakComponent() {
    CommandComponent.call(this, 'LINEBREAK', false);
}
LinebreakComponent.prototype = Object.create(CommandComponent.prototype);
LinebreakComponent.prototype.constructor = LinebreakComponent;
LinebreakComponent.prototype.getSignText = function() {
    return "";
}

function GroupComponent(components) {
    Component.call(this);
    
    var parentMethods = { destroy: this.destroy };
    var that = this;
    
    if (components === undefined) {
        components = [];
    }
    if (typeof(components) === "string") {
        components = SequenceCodec.decodeFromString(components);
    }
    
    this.createInsideHTMLElement = function(containerElem, elem) {
        containerElem.addClass("component-group");
        $.each(components, function(i, component) {
            elem.append(component.getHTMLElement());
            if (component.activatePopover) {
                component.activatePopover();
            }
            // Änderungen weitergeben
            $(component).on('change', function() {
                $(that).trigger('change');
            });
        });
    };
    
	this.destroy = function() {
	    $.each(components, function(i, component) {
	        component.destroy();
	    });
	    parentMethods.destroy.apply(that, arguments);
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

function TwitterComponent() {
    CommandComponent.call(this, 'Twitterkomponente', false);
}

TwitterComponent.prototype = Object.create(CommandComponent.prototype);
TwitterComponent.prototype.constructor = TwitterComponent;
TwitterComponent.prototype.getSignText = function() {
	 return get_twitter_str();
};

// ComponentInfo
// ==============

function ComponentInfo(command, factory, options) {
    if (options === undefined) {
        options = {};
    }

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

// Registrierung der Komponenten
// =============================
var ASC333Components = {
	register : function() {
	    var textComponentInfo = ComponentMapper.registerComponentInfo(new ComponentInfo('TEXT', function(t) {
			return new TextComponent(t);
		}));
		Toolbox.registerToolInfo(new ToolInfo('text', textComponentInfo, {
		    toolType: 'htmlElement',
		    toolHTMLElement: $('<div/>')
		}));
		var twitter = [
			[ 'Twitterkomponente' , "Ruft die letzten Tweets ab" ],
			];
		var openAnimations = [
		    [ 'LEFT', "Von rechts nach links" ],
		    [ 'RIGHT', "Von links nach rechts" ],
		    [ 'UP', "Von unten nach oben" ],
		    [ 'DOWN', "Von oben nach unten" ],
		    [ 'DOFF', "Zeichen erscheinen rechts und bewegen sich einzeln nach links" ],
		    [ 'DOBIG', "Große Zeichen erscheinen rechts und bewegen sich einzeln nach links, werden dabei klein" ],
		    [ 'FLASH', "Mitteilung blinkt" ],
		    [ 'JUMP',  "Mitteilung erscheint sofort" ],
		    [ 'OPENMID', "Mitteilung erscheint in der Mitte und breitet sich nacheinander zu den Seiten aus" ],
		    [ 'OPENRIGHT', "Mitteilung erscheint einzeln von links nach rechts" ],
		    [ 'RANDOM', "Zufälliger Effekt" ],
		    [ 'SHIFTMID', "Mitteilung bewegt sich von der Mitte gleichmäßig zu beiden Seiten" ],
		    [ 'SNOW', "Mitteilung erscheint in Einzelpunkten auf der Anzeige" ]
		];
		var closeAnimations = [
		    [ 'CLOSEMID', "Mitteilung verschwindet von beiden Seiten einzeln zur Mitte hin" ],
		    [ 'CLOSERIGHT', "Mitteilung verschwindet einzeln von links nach rechts" ],
		    [ 'SQUEEZEMID', "Mitteilung bewegt sich von beiden Seiten zur Mitte und verschwindet dort" ],
		    [ 'DSNOW', "Mitteilung verschwindet nach und nach von der Anzeige" ],
		    [ 'AUTOCENTER', "Text seit dem letzten Anfangsbefehl wird automatisch zentriert" ]
		];
		$.each(openAnimations, function(i, animation) {
			var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation[0], undefined, { exchangeableInGroup: 'open_animation' }));
			Toolbox.registerToolInfo(new ToolInfo('open_animation', info, {
				tooltip: animation[1]
			}));
		});
		$.each(twitter, function(i, animation) {
			var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation[0], function(){return new TwitterComponent();}));
			Toolbox.registerToolInfo(new ToolInfo('twitter', info, {
				tooltip: animation[1]
			}));
		});
		$.each(closeAnimations, function(i, animation) {
			var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation[0], undefined, { exchangeableInGroup: 'close_animation' } ));
			Toolbox.registerToolInfo(new ToolInfo('close_animation', info, {
				tooltip: animation[1]
			}));
		});

		var colors = { fg: Config.fgColors, bg: Config.bgColors };
		var colorInfo = {
			fg : ComponentMapper.registerComponentInfo(new ComponentInfo('COLOR', function(c, p) {
				return new ColorCommandComponent('COLOR', c, p);
			})),
			bg : ComponentMapper.registerComponentInfo(new ComponentInfo('BGCOLOR', function(c, p) {
				return new ColorCommandComponent('BGCOLOR', c, p);
			}))
		};
		$.each(['fg', 'bg'], function(i, type) {
			$.each(colors[type], function(j, color) {
				Toolbox.registerToolInfo(new ToolInfo(type + 'color', colorInfo[type], {
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

        var waitInfo = ComponentMapper.registerComponentInfo(new ComponentInfo('WAIT', function(s, p) {
            return new SliderCommandComponent('WAIT', s, p);
        }))
		Toolbox.registerToolInfo(new ToolInfo('pause', waitInfo, {
		    sliderID: 'wait_slider',
		    toolType: 'slider',
		    sliderStep: 1,
		    sliderRange: [1, 9],
		    sliderInitialValue: 5,
		    sliderLabels: ['1s', '9s'],
			overrideFactory : function() { return this.componentInfo.factory($('#wait_slider').val()); }
		}));

		Toolbox.registerToolInfo(new ToolInfo('speed', ComponentMapper.registerComponentInfo(new ComponentInfo('SPEED', function(s) {
			return new SliderCommandComponent('SPEED', s);
		})), {
			sliderID: 'speed_slider',
			toolType: 'slider',
			sliderStep: 1,
			sliderRange: [1, 9],
			sliderInitialValue: 5,
			sliderLabels: ['langsam', 'schnell'],
			overrideFactory : function() { return this.componentInfo.factory($('#speed_slider').val()); }
		}));

		var groupComponentInfo = new ComponentInfo('GROUP', function(c) { return new GroupComponent(c); });
		ComponentMapper.registerComponentInfo(groupComponentInfo);
		var toolboxGroupTextComponent = textComponentInfo.factory();
		toolboxGroupTextComponent.disable();
		// div kann erst erzeugt werden, wenn das HTML-Element schon auf der Seite eingebunden ist
		$(Toolbox).on('created', function() {
		    toolboxGroupTextComponent.addDivCover();
		});
        var toolboxGroupComponent = groupComponentInfo.factory([
            colorInfo.fg.factory(),
            colorInfo.bg.factory(),
            ComponentMapper.getComponentInfoForCommand('LEFT').factory(),
            toolboxGroupTextComponent,
            waitInfo.factory()
        ]);
		Toolbox.registerToolInfo(new ToolInfo('standard', groupComponentInfo, {
		   // toolText : 'Standardelement',
		    toolType: 'htmlElement',
		    toolHTMLElement: toolboxGroupComponent.getHTMLElement(),
		    overrideFactory: function() {
		        return SequenceCodec.decodeFromString(SequenceCodec.encodeToString([toolboxGroupComponent], false))[0];
		    }
		}));
		
		var linebreakComponentInfo = new ComponentInfo('LINEBREAK', function() { return new LinebreakComponent(); });
		ComponentMapper.registerComponentInfo(linebreakComponentInfo);
        Toolbox.registerToolInfo(new ToolInfo('linebreak', linebreakComponentInfo, {
            //toolText : 'Umbruch'
            tooltip: 'In der Sequenzliste zur besseren Übersicht einen Zeilenumbruch einfügen. Dies hat keine Auswirkung auf die Anzeige auf der Laufschrift!'
        }));
		
		var clocks = ['CLOCK12', 'CLOCK24'];
		$.each(clocks, function(i, clock) {
			var clockInfo = ComponentMapper.registerComponentInfo(new ComponentInfo(clock));
			Toolbox.registerToolInfo(new ToolInfo('clock', clockInfo));
		});
	}
};

