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
  checkStr("<TEXT test><LEFT><TEXT asdefdings><COLOR r><TEXT bld>");
  checkStr("<TEXT test><LEFT>");
  checkStr("<TEXT test><LEFT><TEXT aaa><COLOR g>");
  checkStr("<TEXT test\\<LEFT\\>>");
  checkStr("<TEXT test><GROUP <UP><TEXT  \\>>><TEXT a\\\\\\<><RIGHT><TEXT \\>b>");
  checkStr("<COLOR r><GROUP <COLOR y><BGCOLOR b><TEXT bla\\<LEFT\\>\\\\>><TEXT dings>");
  checkStr("<TEXT \\<>");
  checkStr("<TEXT \\>>");
  checkStr("<TEXT \\\\><LEFT>");
  checkStr("<TEXT a><LEFT><TEXT \\>>");
  checkStr("<TEXT a><LEFT><TEXT \\<>");*/
  
  //SequenceControl.loadSequenceFromText('<GROUP <COLOR y><BGCOLOR b><LEFT>bla<WAIT 5>><COLOR r>test');
  //SequenceControl.loadSequenceFromText('<COLOR r>');
});
