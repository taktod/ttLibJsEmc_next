var source = ["src/c/decoder/opusDecoder.c"];
var func = {
  _opusDecoder_make:   "_opusDecoder_make",
  _opusDecoder_decode: "_opusDecoder_decode",
  _opusDecoder_close:  "_opusDecoder_close"
};
var cflags = ["-Inative/libs/include"];
var ldflags = ["-Lnative/libs/lib/", "-lttLibC", "-lopus"];

/**
 * opusのdecoder
 */
export class OpusDecoder {
  private decoder:number;
  private buf:Uint8Array;
  /**
   * コンストラクタ
   * @param sampleRate
   * @param channelNum
   */
  constructor(
      sampleRate:number,
      channelNum:number) {
    this.decoder = Module[func._opusDecoder_make](
      sampleRate,
      channelNum);
    this._allocate(65536);
  }
  private _allocate(size:number):void {
    var bufPtr:number = Module._malloc(size);
    this.buf = Module.HEAPU8.subarray(bufPtr, bufPtr + size);
  }
  /**
   * デコードを実施する
   * @param opus     変換対象のopusフレーム
   * @param pts      opusフレームのpts値
   * @param timebase opusフレームのtimebase値
   * @param callback デコード後のデータを受け取るcallback
   */
  public decode(
      opus:Uint8Array,
      pts:number,
      timebase:number,
      callback:{(pcm:Int16Array):boolean}):boolean {
    if(this.decoder == 0) {
      return false;
    }
    if(opus == null) {
      return true;
    }
    var length:number = opus.length;
    var srcOpus:Uint8Array = null;
    if(opus.buffer == Module.HEAPU8.buffer) {
      // すでにheapに乗っているデータの場合は、そのまま利用することができる。
      srcOpus = opus;
    }
    else {
      if(length > this.buf.length) {
        console.log("より大きなサイズが必要になったので、reallocする"); 
        Module._free(this.buf.byteOffset);
        this._allocate(length);
      }
      this.buf.set(opus);
      srcOpus = this.buf.subarray(0, length);
    }
    var funcPtr:number = Module.Runtime.addFunction((
        pcmPtr:number,
        pcmSize:number) => {
      return callback(Module.HEAP16.subarray(pcmPtr / 2, (pcmPtr + pcmSize) / 2));
    });
    var result:boolean = Module[func._opusDecoder_decode](
      this.decoder,
      srcOpus.byteOffset,
      length,
      pts & 0xFFFFFFFF,
      (pts >> 32) & 0xFFFFFFFF,
      timebase,
      funcPtr);
    Module.Runtime.removeFunction(funcPtr);
    return result;
  }
  /**
   * デコーダーを閉じます。
   */
  public close():void {
      if(this.decoder == 0) {
          return;
      }
      Module[func._opusDecoder_close](this.decoder);
      Module._free(this.buf.byteOffset);
      this.decoder = 0;
  }
}
