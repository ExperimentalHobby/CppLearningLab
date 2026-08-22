// 汎用CSVパース/シリアライズ関数。標準ライブラリのみを使い、DBには一切依存しない
// (05_STLContainersAlgorithmsのcsv_utilsと同じ「小さく再利用可能なユーティリティ」
// という方針)。
//
// RFC 4180に近い最低限のルールをサポートする:
//   - フィールドはカンマ区切り
//   - フィールドをダブルクォートで囲むと、カンマ・改行・ダブルクォート自体を
//     含められる(クォート内の`""`はエスケープされた1個の`"`として扱う)
#pragma once

#include <string>
#include <vector>

namespace csv {

using Row = std::vector<std::string>;

// CSV全体のテキストをパースし、行×列の二次元配列にする。
// クォートされたフィールド内の改行は1つの論理行として扱われる(つまり
// 戻り値の行数は必ずしも入力のテキスト上の行数と一致しない)。
std::vector<Row> ParseCsv(const std::string& content);

// 1行分のフィールド列から、必要に応じてクォートしたCSVの1行(改行なし)を組み立てる。
std::string BuildCsvLine(const Row& fields);

}  // namespace csv
