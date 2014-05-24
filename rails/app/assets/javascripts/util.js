"use strict";

var FlashMessage = {
  success: function(text) {
    $.achtung({message: text, timeout:5, className:'achtungSuccess'});
  },
  
  error: function(text) {
    $.achtung({message: text, className:'achtungFail'});
  }
};

var StringUtil = {
  indexOfAny: function(str, chars, firstIndex) {
    var first = null;
    for (var i = 0; i < chars.length; i++) {
      var index = str.indexOf(chars[i], firstIndex);
      if (index !== -1 && (first === null || index < first)) {
        first = index;
      }
    }
    
    return (first === null) ? -1 : first;
  }
};
