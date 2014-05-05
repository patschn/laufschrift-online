"use strict";

var SequenceControl = {
  // Die momentan geladene Sequenz
  currentSequence: null,

  _init: function() {
    this.currentSequence = new Sequence;
    
    $("#sequence_new").click($.proxy(this.createNew, this));
    $("#sequence_load").click($.proxy(this.load, this));
    $("#sequence_save").click($.proxy(this.save, this));
    $("#sequence_save_as").click($.proxy(this.saveAs, this));
    $("#sequence_delete").click($.proxy(this.destroy, this));
  },

  // Hilfsfunktionen
  // Gibt die ID der momentan ausgewählten Sequenz zurück, oder undefined falls
  // nichts ausgewählt ist
  getSelectedSequenceID: function() {
    return HTMLAccess.sequenceSelect.find("option:selected").val();
  },
  
  // Lädt einen Sequenz-String
  loadSequenceFromText: function(text) {
    HTMLAccess.commitTextField.val(text);
  },

  // Event-Handler
  createNew: function() {
    this.loadSequenceFromText("");
    this.currentSequence = new Sequence;
  },

  load: function() {
    var loadID = this.getSelectedSequenceID();
    if (typeof loadID === "undefined") {
      return;
    }
    var loadedSequence = new Sequence;
    loadedSequence.load(loadID).done($.proxy(function() {
      this.currentSequence = loadedSequence;
      this.loadSequenceFromText(this.currentSequence.text);
      FlashMessage.success("Sequenz »" + this.currentSequence.title + "« geladen");
    }, this)).fail(function(e) {
      FlashMessage.error("Laden fehlgeschlagen: " + e);
    });
  },

  save: function() {
    // Wenn die Sequenz noch nicht gespeichert wurde, ist das Verhalten wie »Speichern als«
    if (!this.currentSequence.existsOnServer()) {
      this.saveAs();
      return;
    }
    this.currentSequence.text = HTMLAccess.commitTextField.val();
    this.currentSequence.update().done(function() {
      FlashMessage.success("Sequenz gespeichert");
    }).fail(function(e) {
      FlashMessage.error("Speichern fehlgeschlagen: " + e);
    });
  },

  saveAs: function() {
    var title = prompt("Bitte geben Sie einen Sequenznamen ein", (this.currentSequence.existsOnServer()) ? this.currentSequence.title : "");
    if (title == null || title == "") {
      return;
    }
    var seq = this.currentSequence;
    seq.text = HTMLAccess.commitTextField.val();;
    seq.title = title;
    seq.create().done($.proxy(function() {
        $("<option/>", { value: seq.getID(), text: seq.title } ).appendTo(HTMLAccess.sequenceSelect);
        FlashMessage.success("Sequenz gespeichert");
    }, this)).fail(function(e) {
      FlashMessage.error("Speichern fehlgeschlagen: " + e);
    });
  },

  destroy: function(){
    var destroyID = this.getSelectedSequenceID();
    if (typeof destroyID === "undefined") {
      return;
    }
    destroySequence = new Sequence;
    destroySequence.load(destroyID).done($.proxy(function() {
      destroySequence.destroy().done($.proxy(function() {
        HTMLAccess.sequenceSelect.find("option[value=" + destroyID + "]").remove();
        if (this.currentSequence.existsOnServer() && this.currentSequence.getID() == destroyID) {
          this.currentSequence = new Sequence;
        }
        FlashMessage.success("Sequenz gelöscht");
      }, this)).fail(function(e) {
        FlashMessage.error("Löschen fehlgeschlagen: " + e);
      });
    }, this)).fail(function(e) { FlashMessage.error("Löschen fehlgeschlagen: " + e); });
  }
}
