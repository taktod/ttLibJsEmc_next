var source = ["src/c/encoder/theoraEncoder.c"];
var func = {
  _theoraEncoder_make:   "_theoraEncoder_make",
  _theoraEncoder_encode: "_theoraEncoder_encode",
  _theoraEncoder_close:  "_theoraEncoder_close"
};
var cflags = ["-Inative/libs/include"];
var ldflags = ["-Lnative/libs/lib/", "-lttLibC", "-ltheora", "-logg"];

/**
 * yuvデータをtheoraに変換するエンコーダー
 */
export class TheoraEncoder {
  private encoder:number;
  private width:number;
  private height:number;
  private buf:Uint8Array; // ここにメモリーデータを上書きする。
  /**
   * コンストラクタ
   * @param width
   * @param height
   */
  constructor(
      width:number,
      height:number) {
    this.encoder = Module[func._theoraEncoder_make](
      width,
      height);
    this.width = width;
    this.height = height;
    this._allocate(width * height * 3 / 2);
  }
  private _allocate(size:number):void {
    var bufPtr:number = Module._malloc(size);
    this.buf = Module.HEAPU8.subarray(bufPtr, bufPtr + size);
    // このbufにwebGLからpixel情報を更新してやればよいという話になる。
  }
  /**
   * 変換bufferを参照する
   */
  public refBuffer():Uint8Array {
    if(this.encoder == 0) {
      return null;
    }
    return this.buf;
  }
  /**
   * エンコードを実施する。
   * @param callback 変換後のデータを受け取るcallback
   */
  public encode(callback:{(theora:Uint8Array):boolean}):boolean {
    if(this.encoder == 0) {
      return false;
    }
    var wh:number = this.width * this.height;
    var funcPtr:number = Module.Runtime.addFunction((
        theoraPtr:number,
        theoraLength:number) => {
      return callback(
        Module.HEAPU8.subarray(theoraPtr, theoraPtr + theoraLength)
      );
    });
    var result:boolean = Module[func._theoraEncoder_encode](
      this.encoder,
      this.buf.byteOffset,
      wh,
      this.buf.byteOffset + wh,
      wh >> 2,
      this.buf.byteOffset + wh + (wh >> 2),
      wh >> 2,
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
    Module[func._theoraEncoder_close](this.encoder);
    Module._free(this.buf.byteOffset);
    this.encoder = 0;
  }
}
