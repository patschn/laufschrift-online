"use strict";

var FlashMessage = {
  success: function(text) {
    $.achtung({message: text, timeout:5, className:'achtungSuccess'});
  },
  
  error: function(text) {
    $.achtung({message: text, className:'achtungFail'});
  }
}

var HTMLAccess = {
  // Referenz zum Debug-Textfeld
  commitTextField: null,
  // Referenz zum Sequenz-<select>-Element
  sequenceSelect: null,
  
  // Initialisieren
  _init: function() {
    this.commitTextField = $("#commit_text");
    this.sequenceSelect = $("#sequences");
  }
};
