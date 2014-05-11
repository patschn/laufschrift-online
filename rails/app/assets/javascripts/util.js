"use strict";

var FlashMessage = {
  success: function(text) {
    $.achtung({message: text, timeout:5, className:'achtungSuccess'});
  },
  
  error: function(text) {
    $.achtung({message: text, className:'achtungFail'});
  }
};
