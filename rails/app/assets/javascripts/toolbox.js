"use strict";

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
      groupDef("text", "Textelement"),
      groupDef("clock", "Uhrzeit"),
      groupDef("color", "Farbe"),
      groupDef("charwidth", "Zeichenbreite"),
      groupDef("pause", "Warten"),
      groupDef("speed", "Geschwindigkeit"),
      groupDef("open_animation", "Anfangsanimationen"),
      groupDef("close_animation", "Endanimationen"),
    ];
    var toolInfos = getAllToolInfosByGroup();
    $.each(groups, function(i, group) {
      var groupDiv = $('<div class="tool-group"></div>').appendTo(toolboxDiv);
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
