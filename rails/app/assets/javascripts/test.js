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
}

function TextComponent(text = "") {
	this.text = text;
	this.getText = function() {
		return this.text;
	};
}
TextComponent.prototype = new Component;

function CommandComponent(command) {
	this.getText = function() {
		return "<" + command + ">";
	};
	Object.defineProperties(this, {
		command: { get: function() { return command }, enumerable: true }
	});
}
CommandComponent.prototype = new Component;

function ColorCommandComponent(color = "y") {
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

var blubb = SequenceCodec.decodeFromString("test<LEFT>" + "dings<COLOR r>bla");
console.log(blubb);
console.log(SequenceCodec.encodeToString(blubb));
