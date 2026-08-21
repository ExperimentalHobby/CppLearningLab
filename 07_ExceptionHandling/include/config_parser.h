#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <map>
#include <string>

// key=value形式の設定ファイルを読み込み、key -> valueのマップを返す。
// ファイルが開けない場合はFileOpenError、'='を含まない行がある場合は
// ParseError(行番号付き)を投げる。
std::map<std::string, std::string> ParseConfigFile(const std::string& path);

// 文字列を正の整数に変換する。数値に変換できない場合はParseErrorを投げる
// （std::stoiが投げるstd::invalid_argument/std::out_of_rangeを、
//   呼び出し側にとって意味のある独自例外に変換する）。
int ParsePositiveInt(const std::string& key, const std::string& value);

// portが[1, 65535]の範囲内であることを検証する。範囲外ならValueOutOfRangeErrorを投げる。
void ValidatePort(int port);

#endif  // CONFIG_PARSER_H
