"use strict";

var ComponentMapper = (function() {
  var components = {};
  
  var registerComponentInfo = function(info) {
    components[info.command] = info;
  };
  
  var getAllComponentInfos = function() {
    return components;
  };

  var getComponentInfoForCommand = function(command) {
    if (components[command] === undefined) {
      throw new Error("Component for command '" + command + "' unknown");
    }
    return components[command];
  };

  return {
    registerComponentInfo: registerComponentInfo,
    getAllComponentInfos: getAllComponentInfos,
    getComponentInfoForCommand: getComponentInfoForCommand
  };
})();

function Component() {
  this.getText = function() {
    throw new Error("Pure virtual function");
  };
  
  this._createInsideHTMLElement = function() {
    throw new Error("Pure virtual function");
  };
  
  this.createHTMLElement = function() {
    var elem = $('<div class="component"></div>');
    elem.data("component", this);
    this._createInsideHTMLElement(elem);
    // connectToSortable: '#', helper: 'clone' oder function() 
    //elem.draggable();
    return elem;
  };  
}

function TextComponent(text) {
  if (typeof(text) === "undefined") {
    text = "";
  }
  this.text = text;
  this.getText = function() {
    return this.text;
  };
  
  this._createInsideHTMLElement = function(elem) {
    elem.addClass("component-text");
    var input = $('<input type="text" />');
    input.val(this.text);
    elem.append(input);
  };
}
TextComponent.prototype = new Component;

function CommandComponent(command) {
  this.getText = function() {
    return "<" + command + ">";
  };
  
  this._createInsideHTMLElement = function(elem) {
    elem.addClass("component-command-" + this.command);
    elem.append(this.command);
  };
  
  Object.defineProperties(this, {
    command: { get: function() { return command }, enumerable: true }
  });
}
CommandComponent.prototype = new Component;

function ColorCommandComponent(color) {
  if (typeof(color) === "undefined") {
    color = 'y';
  }
  CommandComponent.call(this, "COLOR");
  this.getText = function() {
    return "<" + this.command + " " + color + ">";
  };
  Object.defineProperties(this, {
    color: { get: function() { return color; }, enumerable: true }
  });
}
ColorCommandComponent.prototype = new CommandComponent;

function ComponentInfo(group,command,factory) {
  if (typeof(factory) === "undefined") {
    factory = function() {
      return new CommandComponent(command);
    }
  }
  
  this.createToolHTMLElement = function() {
    var elem = $('<div class="tool">' + command + '</div>');
    elem.data("component-info", this);
    if (command) {
      elem.addClass("tool-command-" + command);
    }
    // connectToSortable: '#', helper: 'clone' oder function() 
    //elem.draggable();
    return elem;
  };

  Object.defineProperties(this, {
    group: { get: function() { return group; }, enumerable: true },
    command: { get: function() { return command; }, enumerable: true },
    factory: { get: function() { return factory; }, enumerable: true }
  });
}

ComponentMapper.registerComponentInfo(new ComponentInfo('animation', 'LEFT'));
ComponentMapper.registerComponentInfo(new ComponentInfo('color', 'COLOR', function(c) { return new ColorCommandComponent(c); }));

var SequenceCodec = {
  encodeToString: function(components) {
    var str = "";
    $.each(components, function(i, component) {
      str += component.getText();
    });
    return str;
  },

  decodeFromString: function(str) {
    var re = /(.*?)<(?!<)([^>]*?)(?: ([^>]+?))?>/g;
    var parts;
    var components = new Array;
    console.log("Decoding string '" + str + "' to components...");
    var lastSuccessfulIndex = 0;
    while ((parts = re.exec(str)) !== null) {
      var text = parts[1];
      var command = parts[2];
      var commandArgs = parts[3];
      console.log("- decoded part: text '" + text + "' command '" + command + "' args '" + commandArgs + "'");
      if (text.length > 0) {
        components.push(new TextComponent(text));
      }
      
      var commandComponent = ComponentMapper.getComponentInfoForCommand(command).factory(commandArgs);
      components.push(commandComponent);
      lastSuccessfulIndex = re.lastIndex;
    }
    var lastText = str.substr(lastSuccessfulIndex);
    if (lastText.length > 0) {
      console.log("- adding text: '" + lastText + "'");
      components.push(new TextComponent(lastText));
    }
    console.log("-> decoded to " + components.length + " components");
    return components;
  }
};

var Toolbox = (function() {
  var toolboxDiv;
  
  var init = function() {
    toolboxDiv = $('#toolbox-inner');
  };
  
  var dragHelper = function(elem) {
    // Die ComponentInfo als data-Attribut muss erhalten bleiben
    var clone = elem.clone();
    clone.data("component-info", elem.data("component-info"));
    return clone;
  };

  var create = function() {
    var componentInfos = ComponentMapper.getAllComponentInfos();
    $.each(componentInfos, function(command, componentInfo) {
      var elem = componentInfo.createToolHTMLElement();
      toolboxDiv.append(elem);
      elem.draggable({ connectToSortable: '#sequence-inner', helper: function() { return dragHelper(elem); } });
    });
  };
  
  return {
    init: init,
    create: create
  }; 
})();

$(document).ready(function() {
  Toolbox.init();
  Toolbox.create();
  
  var sequenceDiv = $('#sequence-inner');
  // Workaround für http://bugs.jqueryui.com/ticket/7498
  sequenceDiv.append('<div id="sequence-init-helper" class="component"></div>');
  sequenceDiv.sortable({ items: "> div.component", revert: false });
  sequenceDiv.empty().append('<div class="clear">');
  // Wenn von der Toolbox Elemente reingezogen werden, müssen sie noch als Komponenten
  // neu erzeugt werden.
  // Es muss sortbeforestop sein, weil es bei sortstop den Helper nicht mehr gibt,
  // der hat aber unser ComponentInfo!
  sequenceDiv.on("sortbeforestop", function(event, ui) {
    // Helper ist die temporäre Kopie beim Ziehen, die vom Draggable helper erzeugt wird
    // Item ist die unsichtbare Kopie, die hinterher eingefügt wird
    // Nur ersetzen, wenn das Element von der Toolbox kam und component-info hat
    if (ui.helper.data("component-info")) {
      var c = ui.helper.data("component-info").factory();
      ui.item.replaceWith(c.createHTMLElement());
    }
  });
  function componentListFromHTML() {
    return $.map(sequenceDiv.children("div.component"), function(componentDiv) {
      return $(componentDiv).data("component");
    });
  }
  sequenceDiv.on("sortstop", function(event, ui) {
    var text = SequenceCodec.encodeToString(componentListFromHTML());
    HTMLAccess.commitTextField.val(text);
  });
  
  var blubb = SequenceCodec.decodeFromString("test<LEFT>asdefdings<COLOR r>bldings<COLOR r>bladings<COLOR r>bladings<COLOR r>bladings<COLOR r>bladings<COLOR r>bladings<COLOR r>bladings<COLOR r>blaa");
  console.log(blubb);
  console.log(SequenceCodec.encodeToString(blubb));
  $.each(blubb, function(i, component) {
    $('#sequence-inner .clear').before(component.createHTMLElement());
  });
});
