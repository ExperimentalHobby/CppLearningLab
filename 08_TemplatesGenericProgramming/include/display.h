#ifndef DISPLAY_H
#define DISPLAY_H

#include <sstream>
#include <string>

// 関数テンプレートの基本: 任意の型T同士を比較して大きい方への参照を返す。
// intやdoubleなど、operator>が定義されている型であれば同じコードで動作する。
template <typename T>
const T& Max(const T& a, const T& b) {
    return (a > b) ? a : b;
}

// 関数テンプレートの汎用版(プライマリテンプレート): 値をostringstream経由で文字列化する。
// FixedStack/FixedQueueと同様、任意の型でインスタンス化されうる汎用版は
// 定義自体をヘッダーに置く必要がある。
template <typename T>
std::string ToDisplayString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// bool版の明示的特殊化: "1"/"0"ではなく"true"/"false"を返す。
// 明示的特殊化は対象の型が確定しているため、通常の関数と同様に宣言をヘッダーに、
// 定義をsrc/display.cppに分離できる。
template <>
std::string ToDisplayString<bool>(const bool& value);

// std::string版の明示的特殊化: 値をダブルクォートで囲んで返す。
template <>
std::string ToDisplayString<std::string>(const std::string& value);

#endif  // DISPLAY_H
