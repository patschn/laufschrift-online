"use strict";

$(document).ready(function() {
  HTMLAccess.init();
  SequenceControl.init();
  
  $("#commit_form")
    .on("ajax:success", function(e, data, status, xhr) {
      FlashMessage.success('Text an die Laufschrift übergeben');
    })
    .on("ajax:error", function(e, xhr, status, error) {
      FlashMessage.error('Fehler beim Senden an die Laufschrift: ' + error);
    });
});
