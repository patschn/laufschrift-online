"use strict";


$(document).ready(function() {
/* 

  function A() {
      var priv;
      
      this.read = function() {
          console.log("priv is " + priv);
      };
      
      this.changePriv = function() {
          console.log("changePriv()");
          priv = 5;
      };
  }
  A.prototype.read2 = function() {
      console.log("priv is " + priv);
  };

  function B() {
      A.call(this);
      
      var superChangePriv = this.changePriv;
      this.changePriv = function() {
          superChangePriv.apply(this);
      };
  }
  B.prototype = Object.create(A.prototype);
  B.prototype.constructor = B;

  var t = new B();
  var t2 = new B();
  t.read();
  t2.read();
  t.changePriv();
  t.read();
  t2.read();
  */
  function checkStr(str) {
    var components = SequenceCodec.decodeFromString(str);
    var newstr = SequenceCodec.encodeToString(components, false);
    if (newstr !== str) {
      throw new Error("Check failed ('" + str + "' vs '" + newstr + "')");
    }
  }
  
  /*checkStr("<RIGHT>");
  checkStr("test<LEFT>asdefdings<COLOR r>bld");
  checkStr("test<LEFT>");
  checkStr("test<LEFT>aaa<COLOR g>");
  checkStr("test\\<LEFT\\>");
  checkStr("test<GROUP <UP> \\>>a\\\\\\<<RIGHT>\\>b");
  checkStr("<COLOR r><GROUP <COLOR y><BGCOLOR b>bla\\<LEFT\\>\\\\>dings");
  checkStr("\\<");
  checkStr("\\>");
  checkStr("\\\\<LEFT>");
  checkStr("a\<LEFT>\\>");
  checkStr("a<LEFT>\\<");*/
  
  SequenceControl.loadSequenceFromText('<GROUP <COLOR y><BGCOLOR b><LEFT>bla><COLOR r>test');
  //SequenceControl.loadSequenceFromText('<COLOR r>');
});
