# ttlibjsemc_next

# 作者
taktod <https://twitter.com/taktod>

# 概要
ttLibJsEmcのtypescriptを使った場合のreferenceとかやりにくいので、こっちもまとめ直した。
ベースはgulp -> webpackにした。
nodeでの利用は考えないが、typescriptでrequireができるようにはしておく。

動作テストもwebpackでできるようにしておく。

# 利用コマンド

```
npm run test
```

動作テストする。

```
npm run build
```
生成データを作る。

この２つ。

# 利用方法

## webで使う場合

ttLibJsEnc.jsをscriptタグのsrcで読み込むと
tteでアクセスできるようになる。


## typescriptで使う場合

import {tte} from "ttlibjsemc_next";
とすると
ttでアクセスできるようになる。

こんな具合でやってみる

## その他セットアップについて

とりあえずmacだったら
```
$ brew install node
$ brew install automake
$ brew install cmake
$ brew install libtool
$ npm install
```

emscripten関連が必要なので、emsdk(emsdk_portableでしたっけ)が別途必要で、pathを通しておいてください。
$ emcc --helpでemccのヘルプがでてくる感じで

native/makenative.shを実行すると、必要なライブラリをダウンロードして、
emscriptenで使えるようにコンパイルします。

あとは
```
$ npm run build
```
で必要なものができあがり。

# メモ
とりあえずbsdライセンスで使えるものだけ、集めてみた。
soundtouchとかはやってもいいかな。と思わないでもない。
まぁ、lgplだからやらずにおいとくけど。