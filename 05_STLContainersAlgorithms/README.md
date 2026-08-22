# 05. STLコンテナ・アルゴリズム

## 目的
`vector`、`map`、`set`などの標準コンテナと、`<algorithm>`のアルゴリズム関数を使い、
実践的なデータ処理を効率よく書けるようになる。

## 学習ポイント
- `std::vector`, `std::map`, `std::set`, `std::unordered_map` の使い分け
- イテレータの概念
- `sort`, `find`, `count_if`, `accumulate` 等の頻出アルゴリズム
- ラムダ式との組み合わせ

## 推奨ライブラリ/ツール
- 標準ライブラリ（`<vector>`, `<map>`, `<algorithm>`, `<numeric>`）

## 成果物イメージ
CSV風のテキストデータ（例: 学生の点数一覧）を読み込み、集計・ソート・フィルタリングを
行う小さなコマンドラインツール。次の03_Databaseでの実データ処理につながる練習。

## ビルド方法・実行方法

01〜04と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。
サンプルCSV（`data/students.csv`）へのパスはCMakeの`DATA_DIR`コンパイル定義で
絶対パスとして埋め込んでいるため、実行ディレクトリを気にせず動作する。
別のCSVファイルを使いたい場合は第1引数でパスを指定できる。

```sh
# 05_STLContainersAlgorithms ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/STLContainersAlgorithms
# 別のCSVを使う場合
./out/build/x64-debug/STLContainersAlgorithms path/to/other.csv
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `data/students.csv` | サンプルデータ（`name,score`のCSV） | - |
| `include/student.h`/`src/student.cpp` | `Student`構造体、`DetermineGrade` | - |
| `include/csv_utils.h`/`src/csv_utils.cpp` | CSVを`std::vector<Student>`にパース | `std::vector`の基本 |
| `include/stats.h`/`src/stats.cpp` | `SortByScoreDescending` | `std::sort`+ラムダ式 |
| 〃 | `CountAtLeast`/`FilterAtLeast` | `std::count_if`/`std::copy_if`+ラムダ式 |
| 〃 | `SumScores`/`AverageScore` | `std::accumulate` |
| 〃 | `GroupByGrade` | `std::map`（キー順に走査したい場合） |
| 〃 | `CountByGrade` | `std::unordered_map`（順序を気にせず集計したい場合） |
| 〃 | `FindByName` | `std::find_if`とイテレータ（`end()`との比較） |
| `src/main.cpp` | 上記を一通り呼び出し結果を表示 | - |
