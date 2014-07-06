"use strict";

$(document).ready(function() {
  Toolbox.init();
  SequenceControl.init();
  ASC333Components.register();
  Toolbox.create();
  get_tweets();

  $.preload(Config.preloadAssets);
});
