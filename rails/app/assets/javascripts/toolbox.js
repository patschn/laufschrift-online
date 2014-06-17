"use strict";

// ToolInfo
// ========

function ToolInfo(group, componentInfo, options) {
	if (options === undefined) {
		options = {};
	}

    // Standardargumente
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
		return factoryToUse.apply(this, extraArgs);
	};

	this.createToolHTMLElement = function() {
		var text = '';//componentInfo.command;
		if (options.toolText !== undefined) {
			text = options.toolText;
		}
		
		var elem = $('<div class="tool"/>');
		elem.data("tool-info", this);
		if (componentInfo.command) {
			elem.addClass("tool-command-" + componentInfo.command);
		}
		if (options.tooltip !== undefined) {
		    elem.attr('title', options.tooltip);
		    elem.tooltip({ hide: 100, show: 100, tooltipClass: 'tooltip' });
		}
		
		if (options.toolType === 'slider') {
		    var sliderElem = $('<input type="range" />');
		    sliderElem.attr("min", options.sliderRange[0]);
		    sliderElem.attr("max", options.sliderRange[1]);
		    sliderElem.attr("step", options.sliderStep);
		    sliderElem.attr("value", options.sliderInitialValue);
		    sliderElem.attr("id", options.sliderID);
		    var sliderContainer = $('<div class="tool-slider"/>');
		    sliderContainer.append(sliderElem);
		    sliderContainer.append($('<span class="tool-slider-lower-limit"/>').append(options.sliderLabels[0]));
		    sliderContainer.append($('<span class="tool-slider-upper-limit"/>').append(options.sliderLabels[1]));
		    var sliderButton = $('<div class="button-toolbox"/>');
		    sliderButton.append(options.sliderInitialValue);
		    if (componentInfo.command) {
    		    sliderButton.addClass("button-command-" + componentInfo.command);
    		}
		    sliderElem.on("change", function() {
		       sliderButton.text($(this).val());
		    });
		    sliderContainer.append(sliderButton);
		    elem.append(sliderContainer);
        } else if (options.toolType === 'htmlElement') {
            elem.append(options.toolHTMLElement);
        } else {
            elem.append(text);

		    if (options.toolType === "button") {
        		elem.addClass("button-toolbox");
        		if (componentInfo.command) {
            		elem.addClass("button-command-" + componentInfo.command);
                }
        	}
        }
        
		if (options.extraClass !== undefined) {
			elem.addClass(options.extraClass);
		}
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
		},
		options: {
		    get : function() {
		        return options;
		    },
		    enumerable : true
		}
	});
}

// Toolbox
// =======

var Toolbox = (function() {
  var toolboxDiv = null;
  var toolInfosByGroup = {};
  
  var init = function() {
    toolboxDiv = $('#toolbox-inner');
  };
  
  var registerToolInfo = function(info) {
    var groupList = toolInfosByGroup[info.group];
    if (groupList === undefined) {
      groupList = toolInfosByGroup[info.group] = [];
    }
    groupList.push(info);
    return info;
  };
  
  var getAllToolInfosByGroup = function() {
    return toolInfosByGroup;
  };
  
  var getToolInfosForGroup = function(group) {
    return toolInfosByGroup[group];
  };
  
  var dragHelper = function(elem) {
    // Die ToolInfo als data-Attribut muss erhalten bleiben
    var clone = elem.clone();
    clone.data("tool-info", elem.data("tool-info"));
    return clone;
  };

  var create = function() {
    function groupDef(name, text) { return { name: name, text: text }; }
    var groups = [
      groupDef("standard", "Standardelement"),
      groupDef("text", "Textelement"),
      groupDef("charwidth", "Zeichenbreite"),
      groupDef("bgcolor", "Hintergrundfarbe"),
      groupDef("fgcolor", "Textfarbe"),
      groupDef("pause", "Wartezeit"),
      groupDef("speed", "Geschwindigkeit"),
      groupDef("clock", "Uhrzeit"),
      groupDef("linebreak", "Zeilenumbruch"),
      groupDef("open_animation", "Anfangsanimationen"),
      groupDef("close_animation", "Endanimationen"),
	  groupDef("twitter", "Twitter"),
    ];
    var toolInfos = getAllToolInfosByGroup();
    $.each(groups, function(i, group) {
      var groupDiv = $('<div class="tool-group"></div>').appendTo(toolboxDiv);
      groupDiv.addClass('tool-group-' + group.name);
      var heading = $('<h3></h3>').append(group.text).appendTo(groupDiv);
      var groupTools = toolInfos[group.name];
      if (groupTools !== undefined) {
        $.each(groupTools, function(j, tool) {
          var elem = tool.createToolHTMLElement();
          elem.draggable({ connectToSortable: SequenceControl.getSequenceSortable(), helper: function() { return dragHelper(elem); } });
          groupDiv.append(elem);
        });
      }
    });
  };
  
  return {
    init: init,
    registerToolInfo: registerToolInfo,
    getAllToolInfosByGroup: getAllToolInfosByGroup,
    getToolInfosForGroup: getToolInfosForGroup,
    create: create
  }; 
}());
