"use strict";

$(document).ready(function() {
/*  function TestDingsi() {
    var a = null;
    this.doA = function() { a = 5; };
    this.getA = function(){ return a; };
  }
  
  var blubb = new TestDingsi();
  var blubb2 = new TestDingsi();
  console.log(blubb.getA());
    console.log(blubb2.getA());
  blubb.doA();
  console.log(blubb.getA());
    console.log(blubb2.getA());*/
//  var blubb = SequenceCodec.decodeFromString("test<LEFT>asdefdings<COLOR r>bldings<COLOR r>bladings<COLOR r>bladings<COLOR r>bladings<COLOR r>bladings<COLOR r>bladings<COLOR r>bladings<COLOR r>blaa");
//  console.log(blubb);
//  console.log(SequenceCodec.encodeToString(blubb));
  function checkStr(str) {
    var components = SequenceCodec.decodeFromString(str);
    var newstr = SequenceCodec.encodeToString(components, false);
    if (newstr !== str) {
      throw new Error("Check failed ('" + str + "' vs '" + newstr + "')");
    }
  }
  
  checkStr("<RIGHT>");
  checkStr("test<LEFT>asdefdings<COLOR r>bld");
  checkStr("test<LEFT>");
  checkStr("test<LEFT>aaa<COLOR g>");
  checkStr("test\\<LEFT\\>");
  checkStr("test<GROUP <UP> \\>>a\\\\\\<<RIGHT>\\>b");
  checkStr("\\<");
  checkStr("\\>");
  checkStr("a\<LEFT>\\>");
  checkStr("a<LEFT>\\<");
});
