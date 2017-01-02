var source = ["src/c/test.c"];
var func = {_test: "_test"};
var cflags = [];
var ldflags = [];

console.log("test hogehoge");
export class Test {
  constructor(){

  }
  public test():void {
    Module[func._test];
  }
}
