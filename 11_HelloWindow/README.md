# 11. ウィンドウ表示の基本

## 目的
GUIアプリケーションの最小構成として、空のウィンドウを1枚表示する。
イベントループ(メッセージループ)という概念を理解する。

## 学習ポイント
- ウィンドウの作成・表示・破棄のライフサイクル
- イベントループ(メッセージループ)の役割
- GUIライブラリの初期化と終了処理

## 推奨ライブラリ/ツール
- Qt6 (`QApplication` + `QWidget`)、または wxWidgets、または Win32 API (`CreateWindow`)

本課題では**Win32 API**を採用する。理由はルート [README.md](../README.md#guiライブラリの選定について) を参照。

## 成果物イメージ
タイトルバーに任意の文字列を表示した、何もウィジェットを持たない空のウィンドウを
開き、閉じるボタンで終了できるアプリケーション。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 11_HelloWindow ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/HelloWindow
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `RegisterClassExW`/`CreateWindowExW` | ウィンドウクラスの登録とウィンドウ作成 | ウィンドウの作成・表示・破棄のライフサイクル |
| `WndProc`の`WM_DESTROY`→`PostQuitMessage` | ウィンドウ破棄の通知 | 〃 |
| `GetMessage`/`TranslateMessage`/`DispatchMessage`のループ | メッセージループ | イベントループの役割 |
| `wWinMain`エントリポイント、`WIN32`実行ファイル | コンソールを伴わないGUIアプリの起動 | GUIライブラリの初期化と終了処理 |
