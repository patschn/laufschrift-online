"use strict";

var SequenceCodec = {
  escapeText: function(str) {
    str = str.replace(/\\/g, "\\\\");
    str = str.replace(/</g, "\\<");
    str = str.replace(/>/g, "\\>");
    return str;
  },
  
  unescapeText: function(str) {
    str = str.replace(/\\</g, '<');
    str = str.replace(/\\>/g, '>');
    str = str.replace(/\\\\/g, "\\");
    return str;
  },

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
    var components = [];
    console.log("Decoding string '" + str + "' to components...");
    var lastSuccessfulIndex = 0;
    var text, command, commandArgs, commandWithArgs, argSeparatorIndex, commandComponent, openIndex, closeIndex, lastMatchIndex;
    
    while ((openIndex = StringUtil.indexOfAny(str, ['<', "\\"], lastMatchIndex)) !== -1) {
      lastMatchIndex = openIndex + 1;
      // Escape-Zeichen -> weiter
      if (str.charAt(openIndex) === "\\") {
        lastMatchIndex++; // das Zeichen nach dem Escape-Zeichen überspringen
        continue;
      }
      
      var closeFound = false;
      var level = 1;
      // weiter in die Zeichenketten hineinsteigen, aber nur die oberste Ebene
      // dann weiterverarbeiten.
      // Damit wird <GROUP <COLOR r>> dann nur als GROUP-Befehl mit dem Argument
      // "<COLOR r>" verarbeitet. Die Komponente kann dann nochmal diese Funktion
      // aufrufen, falls sie es nochmal aufgeteilt braucht
      while ((closeIndex = StringUtil.indexOfAny(str, ['<','>', "\\"], lastMatchIndex)) !== -1) {
        lastMatchIndex = closeIndex + 1;
        // Escape-Zeichen -> weiter
        if (str.charAt(closeIndex) === "\\") {
          lastMatchIndex++; // das Zeichen nach dem Escape-Zeichen überspringen
          continue;
        } else {
          if (str.charAt(closeIndex) === '<') {
            level++;
          } else if (str.charAt(closeIndex) === '>') {
            level--;
          }
          if (level === 0) {
            closeFound = true;
            break;
          }
        }
      }
      
      if (!closeFound) {
        break;
      }
      
      text = this.unescapeText(str.slice(lastSuccessfulIndex, openIndex));
      commandWithArgs = str.slice(openIndex + 1, closeIndex);
      argSeparatorIndex = commandWithArgs.indexOf(' ');
      if (argSeparatorIndex === -1) {
        command = commandWithArgs;
        commandArgs = undefined;
      } else {
        command = commandWithArgs.slice(0, argSeparatorIndex);
        commandArgs = commandWithArgs.slice(argSeparatorIndex + 1);
      }
    
      console.log("- decoded part: text '" + text + "' command '" + command + "' args '" + commandArgs + "'");
      if (text.length > 0) {
        components.push(new TextComponent(text));
      }
      
      commandComponent = ComponentMapper.getComponentInfoForCommand(command).factory(commandArgs);
      components.push(commandComponent);
      lastSuccessfulIndex = closeIndex + 1;
    }
    var lastText = this.unescapeText(str.substr(lastSuccessfulIndex));
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
  
  var deleteComponent = function(elem) {
    elem.data('component').destroy();
//    elem.remove();
    updateCommitTextField();
  };
  
  var decoratedComponentHTML = function(component) {
    var elem = component.getHTMLElement();
    $(component).on("change", updateCommitTextField);
    var moveHandle = $('<div></div>');
    moveHandle.addClass('component-move-handle');
    elem.append(moveHandle);
    var deleteButton = $('<button></button>');
    deleteButton.addClass('component-delete-button');
    elem.append(deleteButton);
    /*deleteButton.button({
      icons: { primary: "ui-icon-trash" },
      text: false
    });*/
    deleteButton.click(function() { deleteComponent(elem); });
    return elem;
  };
  
  // Lädt einen Sequenz-String
  var loadSequenceFromText = function(text) {
    // Alte Komponenten löschen
    sequenceDiv.children("div.component").each(function(i, component) {
        $(component).data('component').destroy();
    });
    var components = SequenceCodec.decodeFromString(text);
    $.each(components, function() {
      var elem = decoratedComponentHTML(this); 
      sequenceDiv.append(elem);
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
    if (confirm('Wollen Sie wirklich diese Sequenz löschen?')) {
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
    }).fail(function(e) { FlashMessage.error("Löschen fehlgeschlagen: " + e); });}
    else {FlashMessage.error("Löschen abgebrochen");}
  };
  
  var replaceDraggableWithNewComponent = function(event, ui) {
    // Helper ist die temporäre Kopie beim Ziehen, die vom Draggable helper erzeugt wird
    // Item ist die unsichtbare Kopie, die hinterher eingefügt wird (ein Klon des Toolbox-
    // Elements, das kann auch nicht beeinflusst werden)

    // Nur ersetzen, wenn das Element von der Toolbox kam und tool-info hat
    var toolInfo = ui.helper.data("tool-info");
    if (toolInfo) {
      var c = toolInfo.createComponent();
      ui.item.replaceWith(decoratedComponentHTML(c));
    }
  };
  
  var init = function() {
    currentSequence = new Sequence();
    
    commitTextField = $("#commit-text");
    sequenceSelect = $("#sequences");
    sequenceDiv = $('#sequence-inner');
    
    function preventDefaultProxy(fun) { return function(e) { fun(); e.preventDefault(); };  }
    $("#sequence-new").click(preventDefaultProxy(createNew));
    $("#sequence-load").click(preventDefaultProxy(load));
    $("#sequence-save").click(preventDefaultProxy(save));
    $("#sequence-save-as").click(preventDefaultProxy(saveAs));
    $("#sequence-delete").click(preventDefaultProxy(destroy));
   
    // Workaround für http://bugs.jqueryui.com/ticket/7498
    var workaroundHelper = $('<div id="sequence-init-helper" class="component"></div>');
    sequenceDiv.append(workaroundHelper);
    sequenceDiv.sortable({
        items: "> div.component",
        revert: false,
        handle: ".component-move-handle",
        tolerance: "pointer",
        opacity: 0.5, 
        placeholder: 'component-placeholder',
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
    getSequenceSortable: getSequenceSortable,
    loadSequenceFromText: loadSequenceFromText
  };
}());
