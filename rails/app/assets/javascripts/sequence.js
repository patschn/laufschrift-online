"use strict";

// Repräsentiert eine Sequenz mit Titel, Text
// und optional ID, falls die Sequenz auch auf dem Server existiert.
// Mit den entsprechenden Funktionen kann die auf dem Server gespeicherte Sequenz manipuliert werden.
function Sequence() {
  var id = null;
  var self = this;
  this.title = null;
  this.text = null;
  
  this.existsOnServer = function() {
    return (id !== null);
  }
  
  this.load = function(loadID) {
    var dfd = new $.Deferred();
    $.ajax({
      url: Config.sequenceBasePath + '/' + loadID,
      type: 'get'
    }).done(function (data, status, xhr) {
      id = data.id;
      self.title = data.title;
      self.text = data.text;
      dfd.resolve();
    }).fail(function (xhr, status, e) {
      dfd.reject(e);
    });
    return dfd.promise();
  }
  
  this.update = function() {
    if (id === null) {
      throw Error("Sequence not created in database yet");
    }
    var dfd = new $.Deferred();
    $.ajax({
      url: Config.sequenceBasePath + '/' + id,
      type: 'patch',
      data: { sequence: { title: self.title, text: self.text } }
    }).done(function (data, status, xhr) {
      dfd.resolve();
    }).fail(function (xhr, status, e) {
      dfd.reject(e);
    });
    return dfd.promise();
  }
  
  this.create = function() {
    var dfd = new $.Deferred();
    $.ajax({
      url: Config.sequenceBasePath,
      type: 'post',
      data: { sequence: { title: self.title, text: self.text } }
    }).done(function (data, status, xhr) {
      id = data.id;
      dfd.resolve();
    }).fail(function (xhr, status, e) {
      dfd.reject(e);
    });
    return dfd.promise();
  }
  
  this.destroy = function() {
    var dfd = new $.Deferred();
    $.ajax({
      url: Config.sequenceBasePath + '/' + id,
      type: 'delete'
    }).done(function (data, status, xhr) {
      id = null;
      dfd.resolve();
    }).fail(function (xhr, status, e) {
      dfd.reject();
    });
    return dfd.promise();
  }
  
  Object.defineProperties(this, {
    id: { get: function() { return id; }, enumerable: true }
  });
}



