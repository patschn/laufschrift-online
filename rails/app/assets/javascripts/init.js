"use strict";

$(document).ready(function() {
  Toolbox.init();
  SequenceControl.init();
  Toolbox.create();
  
  $("#commit_form")
    .on("ajax:success", function(e, data, status, xhr) {
      FlashMessage.success('Text an die Laufschrift übergeben');
    })
    .on("ajax:error", function(e, xhr, status, error) {
      var errMsg = error;
      if (xhr.status === 500) {
        errMsg = xhr.responseText;
      }
      FlashMessage.error('Fehler beim Senden an die Laufschrift: ' + errMsg);
    });
});
