# 17. 簡易お絵かきアプリ

## 目的
描画API（2Dグラフィックス）とマウスイベントを組み合わせ、
自由線やシンプルな図形を描けるお絵かきアプリを作る。

## 学習ポイント
- 2D描画API（`QPainter`など）の基本
- マウスイベント（press/move/release）を使ったフリーハンド描画
- 描画内容の再描画（`paintEvent`と描画バッファの保持）
- ペン色・太さの変更、クリア機能

## 推奨ライブラリ/ツール
- Qt6 (`QPainter`, `QPixmap`, マウスイベント)

本課題では**Win32 API**を採用する。理由はルート [README.md](../README.md#guiライブラリの選定について) を参照。

## 成果物イメージ
マウスでキャンバス上に自由に線を描け、色・太さの変更とクリアボタンを備えた
簡易ペイントアプリ。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 17_PaintApp ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/PaintApp
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `RecreateCanvas`(`CreateCompatibleDC`/`CreateCompatibleBitmap`) | メモリDC上にキャンバスを保持 | 描画内容の再描画(`paintEvent`と描画バッファの保持) |
| `WM_LBUTTONDOWN`/`WM_MOUSEMOVE`/`WM_LBUTTONUP` + `DrawSegment`(`MoveToEx`/`LineTo`) | マウスドラッグでの自由線描画 | 2D描画API、マウスイベントを使ったフリーハンド描画 |
| `WM_PAINT`での`BitBlt` | メモリDCの内容を画面へコピーするだけ(ダブルバッファリング) | 描画内容の再描画 |
| 色/太さメニュー、`ClearCanvas` | ペン色・太さの変更、クリア機能 | ペン色・太さの変更、クリア機能 |

リサイズ時は簡易実装のため、既存の描画内容を引き継がずキャンバスが白紙に戻る
（`RecreateCanvas`のコメントを参照）。

## 動作確認

ビルド・実行し、外部の自動化スクリプトから`WM_LBUTTONDOWN`/`WM_MOUSEMOVE`/
`WM_LBUTTONUP`を模擬送信して以下を確認済み。

- 白紙状態でのピクセル色が`0xFFFFFF`(白)であること
- マウスドラッグで線を描いた後、その座標のピクセル色が黒(`0x000000`)に変化すること
- メニューから赤色に切り替えて描画すると、ピクセル色が赤(`GetPixel`で`0x0000FF`
  = R=0xFF,G=0,B=0)になること
- 「クリア」メニュー実行後、描画済みの座標も含めて全て白(`0xFFFFFF`)に戻ること
