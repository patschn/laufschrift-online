"use strict";

// ComponentMapper
// ===============

var ComponentMapper = (function() {
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
    registerComponentInfo: registerComponentInfo,
    getAllComponentInfosByCommand: getAllComponentInfosByCommand,
    getComponentInfoForCommand: getComponentInfoForCommand
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
    return text;
  };
  
  this.createInsideHTMLElement = function(containerElem, elem) {
    containerElem.addClass("component-text");
    inputElem = $('<input type="text" />');
    inputElem.val(text);
    var that = this;
    inputElem.change(function() { text = $(this).val(); $(that).trigger('change'); });
    elem.append(inputElem);
  };
  
  Object.defineProperties(this, {
    text: { get: function() { return text; }, set: function(newtext) { text = newtext; if (inputElem) { inputElem.val(newtext); } }, enumerable: true }
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
    command: { get: function() { return command; }, enumerable: true }
  });
}
CommandComponent.prototype = new Component();

function ColorCommandComponent(command, color) {
  CommandComponent.call(this, command);
  
  var that = this;

  if (color === undefined) {
    color = 'y';
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
    color: { get: function() { return color; }, enumerable: true }
  });
}
ColorCommandComponent.prototype = new CommandComponent();


// ComponentInfo
// ==============

function ComponentInfo(command, factory) {
  if (factory === undefined) {
    // Siehe https://stackoverflow.com/questions/1606797/use-of-apply-with-new-operator-is-this-possible
    // Das ruft nur den Konstruktor von CommandComponent auf, übergibt aber sowohl den gespeicherten
    // Befehl als erstes Argument als auch alle Argumente, die an factory() übergeben wurden
    factory = (function() {
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
    command: { get: function() { return command; }, enumerable: true },
    factory: { get: function() { return factory; }, enumerable: true }
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
    if (options.factoryArguments !== undefined) {
      extraArgs = options.factoryArguments;
    }
    return componentInfo.factory.apply(componentInfo, extraArgs);
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
    group: { get: function() { return group; }, enumerable: true },
    componentInfo: { get: function() { return componentInfo; }, enumerable: true }
  });
}


// Registrierung der Komponenten
// =============================
var ASC333Components = {
  register: function() {
    Toolbox.registerToolInfo(new ToolInfo('text', new ComponentInfo(null, function(t) { return new TextComponent(t); }), { toolText: 'Text' }));
    
    var openAnimations = ['LEFT', 'RIGHT', 'UP', 'DOWN', 'DOFF', 'DOBIG', 'FLASH', 'JUMP', 'OPENMID', 'OPENRIGHT', 'RAINBOW', 'RANDOM', 'SHIFTMID', 'SNOW' ];
    var closeAnimations = ['CLOSEMID', 'CLOSERIGHT', 'SQUEEZEMID', 'DSNOW' ];
    $.each(openAnimations, function(i, animation) {
      var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation));
      Toolbox.registerToolInfo(new ToolInfo('open_animation', info));
    });
    $.each(closeAnimations, function(i, animation) {
      var info = ComponentMapper.registerComponentInfo(new ComponentInfo(animation));
      Toolbox.registerToolInfo(new ToolInfo('close_animation', info));
    });

    var colors = [ 'y', 'r', 'g', 'b' ];
    var colorInfo = {
      fg: ComponentMapper.registerComponentInfo(new ComponentInfo('COLOR', function(c) { return new ColorCommandComponent('COLOR', c); })),
      bg: ComponentMapper.registerComponentInfo(new ComponentInfo('BGCOLOR', function(c) { return new ColorCommandComponent('BGCOLOR', c); }))
    };
    $.each(['fg', 'bg'], function(i, type) {
      $.each(colors, function(j, color) {
        Toolbox.registerToolInfo(new ToolInfo('color', colorInfo[type], { extraClass: 'color-' + type + '-' + color, factoryArguments: [color], toolText: (type == 'fg') ? 'A' : '&nbsp;&nbsp;&nbsp;&nbsp;' }));
      });
    });
  }
};
