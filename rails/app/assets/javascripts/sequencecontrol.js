"use strict";

var SequenceCodec = {
  encodeToString: function(components, useSignText) {
    if (useSignText === undefined) {
      useSignText = true;
    }
    var str = "";
    $.each(components, function() {
      str += (useSignText) ? this.getSignText() : this.getAsText();
    });
    return str;
  },

  decodeFromString: function(str) {
    var re = /(.*?)<(?!<)([^>]*?)(?: ([^>]+?))?>/g;
    var parts;
    var components = [];
    console.log("Decoding string '" + str + "' to components...");
    var lastSuccessfulIndex = 0;
    var text, command, commandArgs, commandComponent;
    while ((parts = re.exec(str)) !== null) {
      text = parts[1];
      command = parts[2];
      commandArgs = parts[3];
      console.log("- decoded part: text '" + text + "' command '" + command + "' args '" + commandArgs + "'");
      if (text.length > 0) {
        components.push(new TextComponent(text));
      }
      
      commandComponent = ComponentMapper.getComponentInfoForCommand(command).factory(commandArgs);
      components.push(commandComponent);
      lastSuccessfulIndex = re.lastIndex;
    }
    var lastText = str.substr(lastSuccessfulIndex);
    if (lastText.length > 0) {
      console.log("- adding text: '" + lastText + "'");
      components.push(new TextComponent(lastText));
    }
    console.log("-> decoded to " + components.length + " components");
    return components;
  }
};

var SequenceControl = (function() {
  // Die momentan geladene Sequenz
  var currentSequence = null;
  // Referenz zum Debug-Textfeld
  var commitTextField = null;
  // Referenz zum Sequenz-<select>-Element
  var sequenceSelect = null;
  // Referenz zum Sequenz-<div>
  var sequenceDiv = null;
  
  // Hilfsfunktionen
  // Gibt die ID der momentan ausgewählten Sequenz zurück, oder undefined falls
  // nichts ausgewählt ist
  var getSelectedSequenceID = function() {
    return sequenceSelect.find("option:selected").val();
  };

  var componentListFromHTML = function() {
    return sequenceDiv.children("div.component").map(function() {
      return $(this).data("component");
    });
  };
  
  var currentSaveString = function() {
    return SequenceCodec.encodeToString(componentListFromHTML(), false);
  };
  
  var updateCommitTextField = function() {
    var text = SequenceCodec.encodeToString(componentListFromHTML(), true);
    commitTextField.val(text);
  };
  
  // Lädt einen Sequenz-String
  var loadSequenceFromText = function(text) {
    // Alte Komponenten löschen
    sequenceDiv.children("div.component").remove();
    var component = SequenceCodec.decodeFromString(text);
    $.each(component, function() {
      sequenceDiv.append(this.createHTMLElement());
    });
    updateCommitTextField();
  };

  // Event-Handler
  var createNew = function() {
    loadSequenceFromText("");
    currentSequence = new Sequence();
  };

  var load = function() {
    var loadID = getSelectedSequenceID();
    if (loadID === undefined) {
      return;
    }
    var loadedSequence = new Sequence();
    loadedSequence.load(loadID).done(function() {
      currentSequence = loadedSequence;
      loadSequenceFromText(currentSequence.text);
      FlashMessage.success("Sequenz »" + currentSequence.title + "« geladen");
    }).fail(function(e) {
      FlashMessage.error("Laden fehlgeschlagen: " + e);
    });
  };
  
  var saveAs = function() {
    var title = prompt("Bitte geben Sie einen Sequenznamen ein", (currentSequence.existsOnServer()) ? currentSequence.title : "");
    if (title === null || title === "") {
      return;
    }
    var seq = currentSequence;
    seq.text = currentSaveString();
    seq.title = title;
    seq.create().done(function() {
        $("<option/>", { value: seq.id, text: seq.title } ).appendTo(sequenceSelect);
        sequenceSelect.val(seq.id).change();
        FlashMessage.success("Sequenz gespeichert");
    }).fail(function(e) {
      FlashMessage.error("Speichern fehlgeschlagen: " + e);
    });
  };

  var save = function() {
    // Wenn die Sequenz noch nicht gespeichert wurde, ist das Verhalten wie »Speichern als«
    if (!currentSequence.existsOnServer()) {
      saveAs();
      return;
    }
    currentSequence.text = currentSaveString();
    currentSequence.update().done(function() {
      FlashMessage.success("Sequenz gespeichert");
    }).fail(function(e) {
      FlashMessage.error("Speichern fehlgeschlagen: " + e);
    });
  };

  var destroy = function() {
    var destroyID = getSelectedSequenceID();
    if (destroyID === undefined) {
      return;
    }
    var destroySequence = new Sequence();
    destroySequence.load(destroyID).done(function() {
      destroySequence.destroy().done(function() {
        sequenceSelect.find("option[value=" + destroyID + "]").remove();
        if (currentSequence.existsOnServer() && currentSequence.id === destroyID) {
          currentSequence = new Sequence();
        }
        FlashMessage.success("Sequenz gelöscht");
      }).fail(function(e) {
        FlashMessage.error("Löschen fehlgeschlagen: " + e);
      });
    }).fail(function(e) { FlashMessage.error("Löschen fehlgeschlagen: " + e); });
  };
  
  var replaceDraggableWithNewComponent = function(event, ui) {
    // Helper ist die temporäre Kopie beim Ziehen, die vom Draggable helper erzeugt wird
    // Item ist die unsichtbare Kopie, die hinterher eingefügt wird (ein Klon des Toolbox-
    // Elements, das kann auch nicht beeinflusst werden)

    // Nur ersetzen, wenn das Element von der Toolbox kam und component-info hat
    var compInfo = ui.helper.data("component-info");
    if (compInfo) {
      var c = compInfo.factory();
      ui.item.replaceWith(c.createHTMLElement());
    }
  };
  
  var init = function() {
    currentSequence = new Sequence();
    
    commitTextField = $("#commit_text");
    sequenceSelect = $("#sequences");
    sequenceDiv = $('#sequence_inner');
    
    function preventDefaultProxy(fun) { return function(e) { fun(); e.preventDefault(); };  }
    $("#sequence_new").click(preventDefaultProxy(createNew));
    $("#sequence_load").click(preventDefaultProxy(load));
    $("#sequence_save").click(preventDefaultProxy(save));
    $("#sequence_save_as").click(preventDefaultProxy(saveAs));
    $("#sequence_delete").click(preventDefaultProxy(destroy));
   
    // Workaround für http://bugs.jqueryui.com/ticket/7498
    var workaroundHelper = $('<div id="sequence_init_helper" class="component"></div>');
    sequenceDiv.append(workaroundHelper);
    sequenceDiv.sortable({
        items: "> div.component",
        revert: false,
        // Wenn von der Toolbox Elemente reingezogen werden, müssen sie noch als Komponenten
        // neu erzeugt werden.
        // Es muss sortbeforestop sein, weil es bei sortstop den Helper nicht mehr gibt,
        // der hat aber unser ComponentInfo!
        beforeStop: replaceDraggableWithNewComponent,
        stop: updateCommitTextField
    });
    // Workaround-Hilfselement wieder töten
    workaroundHelper.remove();
  };
  
  var getSequenceSortable = function() {
    return sequenceDiv;
  };
  
  return {
    init: init,
    createNew: createNew,
    load: load,
    save: save,
    saveAs: saveAs,
    destroy: destroy,
    getSequenceSortable: getSequenceSortable
  };
}());
