var source = ["src/c/decoder/theoraDecoder.c"];
var func = {
  _theoraDecoder_make:   "_theoraDecoder_make",
  _theoraDecoder_decode: "_theoraDecoder_decode",
  _theoraDecoder_close:  "_theoraDecoder_close"
};
var cflags = ["-Inative/libs/include"];
var ldflags = ["-Lnative/libs/lib/", "-lttLibC", "-ltheora", "-logg"];

/**
 * theoraのdecoder
 */
export class TheoraDecoder {
  private decoder:number;
  private buf:Uint8Array;
  /**
   * コンストラクタ
   */
  constructor() {
    this.decoder = Module[func._theoraDecoder_make]();
    this._allocate(65536);
  }
  private _allocate(size:number):void {
    var bufPtr:number = Module._malloc(size);
    this.buf = Module.HEAPU8.subarray(bufPtr, bufPtr + size);
  }
  /**
   * デコードを実施する
   * @param theora   変換対象のtheoraフレーム
   * @param callback 変換後のデータを受け取るcallback
   */
  public decode(
      theora:Uint8Array,
      callback:{(y:Uint8Array, yStride:number, u:Uint8Array, uStride:number, v:Uint8Array, vStride:number):boolean}):boolean {
    if(this.decoder == 0) {
      return false;
    }
    if(theora == null) {
      return true;
    }
    var length:number = theora.length;
    var srcTheora:Uint8Array = null;
    if(theora.buffer == Module.HEAPU8.buffer) {
      srcTheora = theora;
    }
    else {
      if(length > this.buf.length) {
        console.log("より大きなサイズが必要になった。");
        Module._free(this.buf.byteOffset);
        this._allocate(length);
      }
      this.buf.set(theora);
      srcTheora = this.buf.subarray(0, length);
    }
    var funcPtr:number = Module.Runtime.addFunction((
        yPtr:number,
        ySize:number,
        yStride:number,
        uPtr:number,
        uSize:number,
        uStride:number,
        vPtr:number,
        vSize:number,
        vStride:number) => {
      return callback(
        Module.HEAPU8.subarray(yPtr, yPtr + ySize), yStride,
        Module.HEAPU8.subarray(uPtr, uPtr + uSize), uStride,
        Module.HEAPU8.subarray(vPtr, vPtr + vSize), vStride
      );
    });
    var result:boolean = Module[func._theoraDecoder_decode](
      this.decoder,
      srcTheora.byteOffset,
      length,
      funcPtr);
    Module.Runtime.removeFunction(funcPtr);
    return result;
  }
  /**
   * 閉じる
   */
  public close() {
    if(this.decoder == 0) {
      return;
    }
    Module[func._theoraDecoder_close](this.decoder);
    Module._free(this.buf.byteOffset);
    this.decoder = 0;
  }
}
