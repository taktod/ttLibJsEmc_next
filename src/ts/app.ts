/// <reference types="@types/jquery" />
console.log("this is test!!");

import {tte} from "./../../";

$(function() {
  console.log("動作開始");
  var opusEncoder = new tte.encoder.OpusEncoder(48000, 2, 480);
  opusEncoder.close();
});
