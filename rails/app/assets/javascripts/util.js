"use strict";

var FlashMessage = {
  success: function(text) {
    $.achtung({message: text, timeout:5, className:'achtungSuccess'});
  },
  
  error: function(text) {
    $.achtung({message: text, className:'achtungFail'});
  }
}

var HTMLAccess = (function() {
  // Referenz zum Debug-Textfeld
  var commitTextField = null;
  // Referenz zum Sequenz-<select>-Element
  var sequenceSelect = null;
  
  // Initialisieren
  var init = function() {
    commitTextField = $("#commit_text");
    sequenceSelect = $("#sequences");
  };
  
  var obj = {
    init: init
  };
  Object.defineProperties(obj, {
    commitTextField: { get: function() { return commitTextField; }, enumerable: true },
    sequenceSelect: { get: function() { return sequenceSelect; }, enumerable: true }
  });
  return obj;
})();
