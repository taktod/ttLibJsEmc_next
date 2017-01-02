var source = ["src/c/encoder/speexEncoder.c"];
var func = {
  _speexEncoder_make:   "_speexEncoder_make",
  _speexEncoder_encode: "_speexEncoder_encode",
  _speexEncoder_close:  "_speexEncoder_close"
};
var cflags = ["-Inative/libs/include"];
var ldflags = ["-Lnative/libs/lib/", "-lttLibC", "-lspeex"];

/**
 * pcmをspeexに変換するエンコーダー
 */
export class SpeexEncoder {
  private encoder:number;
  private buf:Int16Array;
  private bufSampleNum:number;
  private bufChannelNum:number;
  /**
   * コンストラクタ
   * @param sampleRate
   * @param channelNum
   * @param quality
   */
  constructor(
      sampleRate:number,
      channelNum:number,
      quality:number) {
    this.encoder = Module[func._speexEncoder_make](
      sampleRate,
      channelNum,
      quality);
    this.bufChannelNum = channelNum;
    this._allocate(4096);
  }
  private _allocate(sampleNum:number):void {
    this.bufSampleNum = sampleNum;
    var bufSize:number = this.bufChannelNum * this.bufSampleNum * 2;
    var bufPtr:number = Module._malloc(bufSize);
    this.buf = Module.HEAP16.subarray(bufPtr / 2, (bufPtr + bufSize) / 2);
  }
  /**
   * AudioBufferをエンコードする
   * @param buffer   変換対象のAudioBuffer
   * @param callback 変換後のd−たを受け取るcallback
   */
  public encodeBuffer(
      buffer:AudioBuffer,
      callback:{(speex:Uint8Array, pts:number, timebase:number, sampleRate:number, sampleNum:number, channelNum:number):boolean}):boolean {
    if(this.encoder == 0) {
      return false;
    }
    if(buffer == null) {
      return true;
    }
    if(buffer.numberOfChannels != this.bufChannelNum) {
      console.log("入力データのチャンネル数が変化しています。");
      return false;
    }
    var length:number = buffer.getChannelData(0).length;
    if(length > this.bufSampleNum) {
      console.log("より大きなサイズのコピーが必要なので、reallocします。");
      Module._free(this.buf.byteOffset);
      this._allocate(length);
    }
    for(var i = 0;i < buffer.numberOfChannels;++ i) {
      var ary:Float32Array = buffer.getChannelData(i);
      for(var j = 0;j < ary.length;++ j) {
        this.buf[this.bufChannelNum * j + i] = Math.floor(ary[j] * 32767);
      }
    }
    var funcPtr:number = Module.Runtime.addFunction((
        speexPtr:number,
        speexLength:number,
        pts:number,
        pts_high:number,
        timebase:number,
        sampleRate:number,
        sampleNum:number,
        channelNum:number) => {
      return callback(
        Module.HEAPU8.subarray(speexPtr, speexPtr + speexLength),
        pts | (pts_high << 32),
        timebase,
        sampleRate,
        sampleNum,
        channelNum);
    });
    var result:boolean = Module[func._speexEncoder_encode](
      this.encoder,
      this.buf.byteOffset,
      length * this.bufChannelNum * 2,
      funcPtr);
    Module.Runtime.removeFunction(funcPtr);
    return result;
  }
  /**
   * Int16ArrayのPCMデータをエンコードする。
   * @param pcm      変換対象のInt16Array
   * @param callback 変換後のデータを受け取るcallback
   */
  public encodeInt16Array(
      pcm:Int16Array,
      callback:{(speex:Uint8Array, pts:number, timebase:number, sampleRate:number, sampleNum:number, channelNum:number):boolean}):boolean {
    if(this.encoder == 0) {
      return false;
    }
    if(pcm == null) {
      return true;
    }
    // すでにarrayのデータがemscriptenのbuffer上にあるデータの場合はメモリーをコピーする必要ないわけだが・・・
    var length:number = pcm.length;
    var srcPcm:Int16Array = null;
    if(pcm.buffer == Module.HEAP16.buffer) {
      // すでにデータがheapに乗っている場合はそのまま利用することができる。
      srcPcm = pcm; // arrayをそのまま送ればよい
    }
    else {
      if(length > this.buf.length) {
        console.log("より大きなサイズが必要になったので、reallocしなければならない。");
        Module._free(this.buf.byteOffset);
        this._allocate(length / this.bufChannelNum);
      }
      // データをコピーするわけだが・・・
      this.buf.set(pcm); // 対象が大きいのでsetするだけでよい。
      srcPcm = this.buf.subarray(0, length);
    }
    var funcPtr:number = Module.Runtime.addFunction((
        speexPtr:number,
        speexLength:number,
        pts:number,
        pts_high:number,
        timebase:number,
        sampleRate:number,
        sampleNum:number,
        channelNum:number) => {
      return callback(
        Module.HEAPU8.subarray(speexPtr, speexPtr + speexLength),
        pts | (pts_high << 32),
        timebase,
        sampleRate,
        sampleNum,
        channelNum);
    });
    var result:boolean = Module[func._speexEncoder_encode](
      this.encoder,
      srcPcm.byteOffset,
      length * 2,
      funcPtr);
    Module.Runtime.removeFunction(funcPtr);
    return result;
  }
  /**
   * 閉じる
   */
  public close():void {
    if(this.encoder == 0) {
      return;
    }
    Module[func._speexEncoder_close](this.encoder);
    Module._free(this.buf.byteOffset);
    this.encoder = 0;
  }
}
