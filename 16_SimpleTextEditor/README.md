# 16. 簡易テキストエディタ

## 目的
GUIアプリからのファイル入出力（開く/保存/名前を付けて保存）を実装し、
実用アプリに欠かせないファイル操作の扱い方を身につける。

## 学習ポイント
- ファイルダイアログ（開く/保存）の利用
- テキストの読み込み・書き込み（`std::ifstream`/`std::ofstream`または各GUIライブラリのAPI）
- 未保存の変更を検知してタイトルバーやダイアログで警告する仕組み

## 推奨ライブラリ/ツール
- Qt6 (`QTextEdit`, `QFileDialog`)

本課題では**Win32 API**を採用する。理由はルート [README.md](../README.md#guiライブラリの選定について) を参照。

## 成果物イメージ
テキストの編集、新規作成、開く、保存、名前を付けて保存ができる
シンプルなメモ帳アプリ。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 16_SimpleTextEditor ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/SimpleTextEditor
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/text_file.h`/`src/text_file.cpp` | UTF-8ファイルの読み書き(`std::ifstream`/`std::ofstream`)、UTF-16⇔UTF-8変換。ウィンドウ非依存 | テキストの読み込み・書き込み |
| `src/main.cpp`の`ShowOpenDialog`/`ShowSaveAsDialog` | `GetOpenFileNameW`/`GetSaveFileNameW`(comdlg32) | ファイルダイアログの利用 |
| `src/main.cpp`の`OnCommand`(`EN_CHANGE`)/`UpdateTitle`/`ConfirmDiscardChanges` | 変更検知→タイトルバーに`*`表示、新規作成/開く/終了時の保存確認 | 未保存の変更を検知して警告する仕組み |

## 動作確認

- `text_file.h`/`.cpp`（ファイルI/O部分）を一時的な検証用コンソールプログラム
  （リポジトリには含めていない）で検証: 日本語を含むテキストの保存→読み込みが
  完全に一致すること、存在しないファイルの読み込みが正しく失敗を返すことを確認
- GUI側をビルド・実行し、`EnumChildWindows`でエディタ本体(`EDIT`コントロール)の
  構造を確認
- 外部の自動化スクリプトから`WM_CHAR`でキー入力を模擬送信し、`EN_CHANGE`通知に
  よってタイトルバーが`無題 - ...` → `無題 * - ...`に変わる（未保存の変更が
  検知される）ことを確認済み

  補足: `WM_SETTEXT`によるプログラムからのテキスト設定では`EN_CHANGE`は発生しない
  （Win32の仕様）。これはファイルを開いた直後に意図せず「変更あり」状態になるのを
  防ぐのに好都合なため、`g_suppressChangeNotification`フラグは主に将来の変更に
  備えた保険として残している。
