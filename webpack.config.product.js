var webpack = require("webpack");

module.exports = {
  entry:  __dirname + "/src/ts/ttLibJsEmc.ts",
  output: {
    path: __dirname + "/",
    filename: "ttLibJsEmc.js",
    library: ["tte"]
  },
  resolve: {
    extensions: ["", ".js", ".ts"]
  },
  target: 'node',
  module: {
    loaders: [
      {test: /\.emc\.(j|t)s$/, loader: "emscripten-webpack-loader"},
      {test: /\.ts$/, loader: "awesome-typescript-loader"}
    ]
  }
}