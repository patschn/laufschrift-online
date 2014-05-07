"use strict";

var SequenceControl = (function() {
  // Die momentan geladene Sequenz
  var currentSequence = null;

  var init = function() {
    currentSequence = new Sequence;
    
    $("#sequence_new").click(createNew);
    $("#sequence_load").click(load);
    $("#sequence_save").click(save);
    $("#sequence_save_as").click(saveAs);
    $("#sequence_delete").click(destroy);
  };

  // Hilfsfunktionen
  // Gibt die ID der momentan ausgewählten Sequenz zurück, oder undefined falls
  // nichts ausgewählt ist
  var getSelectedSequenceID = function() {
    return HTMLAccess.sequenceSelect.find("option:selected").val();
  };
  
  // Lädt einen Sequenz-String
  var loadSequenceFromText = function(text) {
    HTMLAccess.commitTextField.val(text);
  };

  // Event-Handler
  var createNew = function() {
    loadSequenceFromText("");
    currentSequence = new Sequence;
  };

  var load = function() {
    var loadID = getSelectedSequenceID();
    if (typeof loadID === "undefined") {
      return;
    }
    var loadedSequence = new Sequence;
    loadedSequence.load(loadID).done(function() {
      currentSequence = loadedSequence;
      loadSequenceFromText(currentSequence.text);
      FlashMessage.success("Sequenz »" + currentSequence.title + "« geladen");
    }).fail(function(e) {
      FlashMessage.error("Laden fehlgeschlagen: " + e);
    });
  };

  var save = function() {
    // Wenn die Sequenz noch nicht gespeichert wurde, ist das Verhalten wie »Speichern als«
    if (!currentSequence.existsOnServer()) {
      saveAs();
      return;
    }
    currentSequence.text = HTMLAccess.commitTextField.val();
    currentSequence.update().done(function() {
      FlashMessage.success("Sequenz gespeichert");
    }).fail(function(e) {
      FlashMessage.error("Speichern fehlgeschlagen: " + e);
    });
  };

  var saveAs = function() {
    var title = prompt("Bitte geben Sie einen Sequenznamen ein", (currentSequence.existsOnServer()) ? currentSequence.title : "");
    if (title == null || title == "") {
      return;
    }
    var seq = currentSequence;
    seq.text = HTMLAccess.commitTextField.val();;
    seq.title = title;
    seq.create().done(function() {
        $("<option/>", { value: seq.id, text: seq.title } ).appendTo(HTMLAccess.sequenceSelect);
        FlashMessage.success("Sequenz gespeichert");
    }).fail(function(e) {
      FlashMessage.error("Speichern fehlgeschlagen: " + e);
    });
  };

  var destroy = function() {
    var destroyID = getSelectedSequenceID();
    if (typeof destroyID === "undefined") {
      return;
    }
    var destroySequence = new Sequence;
    destroySequence.load(destroyID).done(function() {
      destroySequence.destroy().done(function() {
        HTMLAccess.sequenceSelect.find("option[value=" + destroyID + "]").remove();
        if (currentSequence.existsOnServer() && currentSequence.id == destroyID) {
          currentSequence = new Sequence;
        }
        FlashMessage.success("Sequenz gelöscht");
      }).fail(function(e) {
        FlashMessage.error("Löschen fehlgeschlagen: " + e);
      });
    }).fail(function(e) { FlashMessage.error("Löschen fehlgeschlagen: " + e); });
  };
  
  return {
    init: init,
    createNew: createNew,
    load: load,
    save: save,
    saveAs: saveAs,
    destroy: destroy
  };
})();
