# 27. CSVインポート/エクスポート

## 目的
DBとCSVファイルの相互変換を実装し、外部データ連携の基本的なパターンを
身につける。

## 学習ポイント
- CSVのパース（区切り文字、引用符、改行の扱い）とシリアライズ
- DBの検索結果をCSVに出力する処理
- CSVを読み込んでDBに一括登録する処理（バッチ処理、エラー行のスキップ等）

## 推奨ライブラリ/ツール
- 標準ライブラリ（`<fstream>`, `<sstream>`）、または軽量CSVパーサーライブラリ
- 21/22で使用したDBライブラリ

本課題ではCSVパースは標準ライブラリのみで自作し(`csv_utils.h`/`.cpp`)、
DBは21番と同じ**SQLite3**（[third_party/sqlite3](../third_party/sqlite3/NOTICE.md)の
amalgamationを共有利用）を使う。

## 成果物イメージ
26番の顧客管理アプリのデータをCSVでエクスポート/インポートできる機能を
コマンドラインまたはGUIメニューとして追加する。

本課題では各課題フォルダを独立してビルド・実行できる構成を保つため、26番の
GUIアプリに機能追加する形ではなく、`products(id, name, price, stock)`という
独立したテーブルを題材にした**コマンドラインツール**として実装した
(DB⇔CSV変換のロジック自体は26番のcustomer_repositoryにもそのまま応用できる)。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 27_CSVImportExport ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/CSVImportExport
```

実行すると、実行ディレクトリに`products_export.csv`(正常系)と
`products_with_errors.csv`(意図的に不正な行を含むCSV)が作成される。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `csv::ParseCsv`/`csv::BuildCsvLine`(`csv_utils.h`/`.cpp`、DB非依存) | クォート内のカンマ・改行・エスケープされた`""`を正しく扱うパース/シリアライズ | CSVのパース（区切り文字、引用符、改行の扱い）とシリアライズ |
| `product_csv::ExportProductsToCsv` | `products`テーブルの全件をヘッダー行付きCSVに出力 | DBの検索結果をCSVに出力する処理 |
| `product_csv::ImportProductsFromCsv` | CSVを1行ずつ`INSERT OR REPLACE`。列数不正/数値列が不正な行はスキップしエラーを記録、処理は継続 | CSVを読み込んでDBに一括登録する処理（バッチ処理、エラー行のスキップ等） |

## 動作確認

ビルド・実行し、以下を確認済み。

1. **クォート処理**: `Widget, Deluxe`(カンマを含む)と`Gadget "Pro"`
   (ダブルクォートを含む)という商品名を含むテーブルをエクスポートすると、
   CSV上でそれぞれ`"Widget, Deluxe"`・`"Gadget ""Pro"""`のように正しく
   クォート・エスケープされること
2. **往復一致**: テーブルをクリアしてエクスポート済みCSVから再インポートすると、
   件数・内容(id/name/price/stock)がエクスポート前と完全に一致すること
3. **エラー行のスキップ**: 価格が数値でない行(`not-a-number`)、列数が5個ある
   壊れた行を含むCSVをインポートすると、その2行はエラーとして記録されつつ
   スキップされ、残り2件の正常な行だけがDBに追加されること
   (`インポート件数: 2, エラー件数: 2`という実行結果と、エラーメッセージに
   行番号と理由が含まれることを確認)
