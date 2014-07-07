"use strict";

var FlashMessage = {
  success: function(text) {
    $.achtung({message: StringUtil.htmlencode(text), timeout:5, className:'achtungSuccess'});
  },
  
  error: function(text) {
    $.achtung({message: StringUtil.htmlencode(text), className:'achtungFail'});
  },
  
  status: function(text) {
    $.achtung({message: StringUtil.htmlencode(text), timeout: 5, className:'achtungStatus'});
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
  },
  
  htmlencode: function(text) {
    return text.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
  },
  
  stripSpecialChars: function(text) {
    return text.replace(/[^a-zA-Z0-9äöüÄÖÜèÉñÑȧȦ!@#$§%^&*();.,\[\]_\-+=':"\/\\§? ]/g, '');
  }
};
