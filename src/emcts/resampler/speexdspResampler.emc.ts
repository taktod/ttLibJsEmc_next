var source = ["src/c/resampler/speexdspResampler.c"];
var func = {
  _speexdspResampler_make:     "_speexdspResampler_make",
  _speexdspResampler_resample: "_speexdspResampler_resample",
  _speexdspResampler_close:    "_speexdspResampler_close"
};
var cflags = ["-Inative/libs/include"];
var ldflags = ["-Lnative/libs/lib/", "-lttLibC", "-lspeexdsp"];

/**
 * speexdspによるリサンプル動作
 */
export class SpeexdspResampler {
  private resampler:number;
  private buf:Int16Array;
  private bufSampleNum:number;
  private bufChannelNum:number;
  /**
   * コンストラクタ
   * @param channelNum
   * @param inputSampleRate
   * @param outputSampleRate
   * @param quality
   */
  constructor(
      channelNum:number,
      inputSampleRate:number,
      outputSampleRate:number,
      quality) {
    this.resampler = Module[func._speexdspResampler_make](
      channelNum,
      inputSampleRate,
      outputSampleRate,
      quality
    );
    this.bufChannelNum = channelNum;
    this._allocate(4096);
  }
  private _allocate(sampleNum:number):void {
    this.bufSampleNum = sampleNum;
    var bufSize:number = this.bufChannelNum * this.bufSampleNum * 2;
    var bufPtr:number = Module._malloc(bufSize);
    console.log("bufPtrをallocします。:" + bufPtr);
    this.buf = Module.HEAP16.subarray(bufPtr / 2, (bufPtr + bufSize) / 2);
    // byteOffsetがそのままbufPtrになる。
  }
  /**
   * AudioBufferをリサンプルします。
   * @param buffer   変換対象のAudioBuffer
   * @param callback 結果データを受け取るcallback
   */
  public resampleBuffer(
      buffer:AudioBuffer,
      callback:{(pcm:Int16Array):boolean}):boolean {
    if(this.resampler == 0) {
      return false;
    }
    if(buffer.numberOfChannels != this.bufChannelNum) {
      console.log("入力データのチャンネル数が変化しています。");
      return false;
    }
    var length:number = buffer.getChannelData(0).length;
    if(length > this.bufSampleNum) {
      console.log("より大きなサイズが必要になったので、reallocしなければならない。");
      Module._free(this.buf.byteOffset);
      this._allocate(length);
    }
    for(var i = 0;i < buffer.numberOfChannels;++ i) {
      var ary:Float32Array = buffer.getChannelData(i);
      for(var j = 0;j < ary.length;++ j) {
        this.buf[this.bufChannelNum * j + i] = Math.floor(ary[j] * 32767);
      }
    }
    var funcPtr:number = Module.Runtime.addFunction((pcmPtr:number, pcmLength:number) => {
      return callback(Module.HEAP16.subarray(pcmPtr / 2, (pcmPtr + pcmLength) / 2));
    });
    var result:boolean = Module[func._speexdspResampler_resample](
      this.resampler,
      this.buf.byteOffset,
      length * this.bufChannelNum * 2,
      funcPtr);
    Module.Runtime.removeFunction(funcPtr);
    return result;
  }
  /**
   * Int16Arrayをリサンプルします。
   * @param pcm      変換対象のInt16Array
   * @param callback 結果データを受け取るcallback
   */
  public resampleInt16Array(
      pcm:Int16Array,
      callback:{(pcm:Int16Array):boolean}):boolean {
    if(this.resampler == 0) {
      return false;
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
    var funcPtr:number = Module.Runtime.addFunction((pcmPtr:number, pcmLength:number) => {
      return callback(Module.HEAP16.subarray(pcmPtr / 2, (pcmPtr + pcmLength) / 2));
    });
    var result:boolean = Module[func._speexdspResampler_resample](
      this.resampler,
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
    if(this.resampler == 0) {
      return;
    }
    Module[func._speexdspResampler_close](this.resampler);
    Module._free(this.buf.byteOffset);
    this.resampler = 0;
  }
}
