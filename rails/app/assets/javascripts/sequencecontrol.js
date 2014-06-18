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
      
      text = str.slice(lastSuccessfulIndex, openIndex);
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
  // Referenz zum Sequenz-<ul>-Element
  var sequenceSelect = null;
  // Referenz zur listasselect-Instanz für sequenceSelect
  var sequenceList = null;
  // Referenz zum Sequenz-<div>
  var sequenceDiv = null;
  // Referenz zum Sequenz-Namen-<div>
  var sequenceNameDiv = null;
  
  // Hilfsfunktionen
  // Gibt die ID der momentan ausgewählten Sequenz zurück, oder undefined falls
  // nichts ausgewählt ist
  var getSelectedSequenceID = function() {
    return sequenceList.getSelectedValue();
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
  
  var updateLinebreaks = function() {
    sequenceDiv.children('br').remove();
    $.each(componentListFromHTML(), function(i, component) {
      if (component instanceof LinebreakComponent) {
        component.getHTMLElement().after($('<br/>'));
      }
    });
  };
  
  var setSequenceName = function(name) {
    sequenceNameDiv.text(name);
  };
  
  var deleteComponent = function(elem) {
    elem.data('component').destroy();
//    elem.remove();
    updateCommitTextField();
  };
  
  var specialCharacterPopover = null;
  var specialCharacterPopoverComponent = null;
  
  var specialCharacterButtonClicked = function(e) {
    // Damit der Fokus auf dem Input-Element bleibt
    e.preventDefault();
         
    //specialCharacterButton.data('popover').hide();
    
    /*var activeElem = $(document.activeElement);
    // Nur Input-Elemente
    if (activeElem.prop('tagName').toLowerCase() !== 'input') {
      return;
      
    // Component-Instanz suchen. parentsUntil gibt das Element, das den Match
    // auslöst, nicht mit zurück, also muss man nochmal parent() machen
    var component = activeElem.parentsUntil('.component').last().parent().data('component');

    }*/
    
    var component = specialCharacterPopoverComponent;
    if (component instanceof GroupComponent) {
      // Text-Komponente innerhalb der Gruppe raussuchen
      component = $.grep(component.components, function(c) {
        return (c instanceof TextComponent);
      })[0];
    }
    if (component === null || !(component instanceof TextComponent)) {
      return;
    }
    
    // Sonderzeichen einfügen
    component.insertTextAtCursor(e.data);
  };
  
  var addSpecialCharacterPopover = function(component) {
    var specialCharacterButton = $('<button></button>');
    specialCharacterButton.addClass('component-special-character-button');
    component.getHTMLElement().append(specialCharacterButton);
    // Nur ein Popover-Element für alle Buttons verwenden
    if (specialCharacterPopover === null) {
      specialCharacterPopover = $('<div></div>');
      $.each(Config.specialCharacters, function(i, specialCharacter) {
        var button = $('<div></div>');
        button.addClass('button-toolbox');
        button.mousedown(specialCharacter, specialCharacterButtonClicked);
        button.text(specialCharacter);
        specialCharacterPopover.append(button);
      });
    	specialCharacterPopover.addClass('popover');
    	specialCharacterPopover.addClass('popover-special-characters');
      $('body').append(specialCharacterPopover);
    }
    specialCharacterButton.popover({ my: 'center top', at: 'center bottom', popover: specialCharacterPopover, collision: 'flipfit flipfit' });
    // Referenz setzen, damit beim Einfügen klar ist für welche Komponente
    // das Ganze eigentlich ist
    specialCharacterButton.on('popover-show', function() {
      specialCharacterPopoverComponent = component;
    });
    // Referenz aufräumen
    specialCharacterButton.on('popover-hide', function() {
      specialCharacterPopoverComponent = null;
    });
    // Fokuswechsel verhindern, damit der Cursor im Textfeld an seiner
    // Position bleibt
    specialCharacterButton.mousedown(function(e) { e.preventDefault(); });
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
    deleteButton.click(function() { deleteComponent(elem); });
    if (component instanceof TextComponent || component instanceof GroupComponent) {
      addSpecialCharacterPopover(component);
    }
    return elem;
  };
  
  // Lädt einen Sequenz-String
  var loadSequenceFromText = function(text) {
    // Alte Komponenten löschen
    $.each(componentListFromHTML(), function(i, component) {
        component.destroy();
    });
    var components = SequenceCodec.decodeFromString(text);
    $.each(components, function() {
      var elem = decoratedComponentHTML(this); 
      sequenceDiv.append(elem);
    });
    updateCommitTextField();
    updateLinebreaks();
  };

  // Event-Handler
  var createNew = function() {
    loadSequenceFromText("");
    setSequenceName('Unbenannte Sequenz');
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
      setSequenceName(currentSequence.title);
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
      sequenceList.appendElement(seq.id, seq.title);
      sequenceList.selectElementByValue(seq.id);
      setSequenceName(title);
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
    if (!currentSequence.modifiable) {
      FlashMessage.error("Demosequenzen können nicht verändert werden");
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
      if (!destroySequence.modifiable) {
        FlashMessage.error("Demosequenzen können nicht gelöscht werden");
        return;
      }
      destroySequence.destroy().done(function() {
        sequenceList.removeElementByValue(destroyID);
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
    sequenceNameDiv = $('#sequence-name');
    
    sequenceList = sequenceSelect.listasselect().data('swpListasselect');
    
    setSequenceName('Unbenannte Sequenz');
    
    function preventDefaultProxy(fun) { return function(e) { fun(); e.preventDefault(); };  }
    $("#sequence-new").click(preventDefaultProxy(createNew));
    $("#sequence-load").click(preventDefaultProxy(load));
    $("#sequence-save").click(preventDefaultProxy(save));
    $("#sequence-save-as").click(preventDefaultProxy(saveAs));
    $("#sequence-delete").click(preventDefaultProxy(destroy));
    $("#play").click(preventDefaultProxy(playClicked));
   
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
    sequenceDiv.on('sortstop', updateLinebreaks);
    // Workaround-Hilfselement wieder töten
    workaroundHelper.remove();
  };
  
  var getSequenceSortable = function() {
    return sequenceDiv;
  };
  
  var playClicked = function() {
    commit(commitTextField.val());
  };
  
  var commit = function(text) {
    $.ajax({
      url: Config.commitPath,
      type: 'post',
      data: { text: text }
    }).done(function (data, status, xhr) {
      FlashMessage.success('Text an die Laufschrift übergeben');
    }).fail(function (xhr, status, e) {
      var errMsg = e;
      if (xhr.status === 500) {
        errMsg = xhr.responseText;
      }
      FlashMessage.error('Fehler beim Senden an die Laufschrift: ' + errMsg);
    });
  };
  
  return {
    init: init,
    createNew: createNew,
    load: load,
    save: save,
    saveAs: saveAs,
    destroy: destroy,
    getSequenceSortable: getSequenceSortable,
    loadSequenceFromText: loadSequenceFromText,
    commit: commit
  };
}());
