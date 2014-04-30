function Sequence() {
  var id = null;
  var self = this;
  this.title = null;
  this.text = null;
  
  this.getID = function() {
    return id;
  }
  
  this.existsOnServer = function() {
    return (id !== null);
  }
  
  this.load = function(loadID, onSuccess, onFail) {
    $.ajax({
      url: Config.sequenceBasePath + '/' + loadID,
      type: 'get'
    }).done(function (data, status, xhr) {
      id = data.id;
      self.title = data.title;
      self.text = data.text;
      onSuccess();
    }).fail(function (xhr, status, e) {
      onFail(e);
    });
  }
  
  this.update = function(onSuccess, onFail) {
    if (id === null) {
      throw Error("Sequence not created in database yet");
    }
    $.ajax({
      url: Config.sequenceBasePath + '/' + id,
      type: 'patch',
      data: { sequence: { title: self.title, text: self.text } }
    }).done(function (data, status, xhr) {
      onSuccess();
    }).fail(function (xhr, status, e) {
      onFail(e);
    });
  }
  
  this.create = function(onSuccess, onFail) {
    $.ajax({
      url: Config.sequenceBasePath,
      type: 'post',
      data: { sequence: { title: self.title, text: self.text } }
    }).done(function (data, status, xhr) {
      id = data.id;
      onSuccess();
    }).fail(function (xhr, status, e) {
      onFail(e);
    });
  }
  
  this.destroy = function(onSuccess, onFail) {
    $.ajax({
      url: Config.sequenceBasePath + '/' + id,
      type: 'delete'
    }).done(function (data, status, xhr) {
      id = null;
      onSuccess();
    }).fail(function (xhr, status, e) {
      onFail(e);
    });
  }
}



