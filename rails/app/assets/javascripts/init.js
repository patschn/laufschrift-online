"use strict";

$(document).ready(function() {
  HTMLAccess._init();
  SequenceControl._init();
  
  $("#commit_form")
    .on("ajax:success", function(e, data, status, xhr) {
      FlashMessage.success('Text an die Laufschrift übergeben');
    })
    .on("ajax:error", function(e, xhr, status, error) {
      FlashMessage.error('Fehler beim Senden an die Laufschrift: ' + error);
    });
});
