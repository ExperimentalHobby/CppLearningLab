// 最小限のJSONパーサー。nlohmann/json等が本開発環境に無いため自作する
// (27_CSVImportExportのCSVパーサーと同じ「小さく自作する」方針)。
//
// RFC 8259の主要な部分(オブジェクト/配列/文字列/数値/真偽値/null、
// 基本的なエスケープシーケンス)をサポートする。コメントや末尾カンマ等の
// 非標準拡張には対応しない。
#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace json {

class JsonParseError : public std::runtime_error {
   public:
    explicit JsonParseError(const std::string& message) : std::runtime_error(message) {}
};

enum class JsonType {
    kNull,
    kBool,
    kNumber,
    kString,
    kArray,
    kObject,
};

class JsonValue {
   public:
    JsonValue() : type_(JsonType::kNull) {}

    JsonType type() const { return type_; }
    bool IsNull() const { return type_ == JsonType::kNull; }
    bool IsObject() const { return type_ == JsonType::kObject; }
    bool IsArray() const { return type_ == JsonType::kArray; }

    bool AsBool() const;
    double AsNumber() const;
    const std::string& AsString() const;
    const std::vector<JsonValue>& AsArray() const;

    // オブジェクトの場合、キーが存在すればその値への参照を返す。
    // 存在しない/オブジェクトでない場合はnullptrを返す(例外を投げない
    // 「安全な」アクセサ。無いフィールドが多いJSON APIレスポンスを扱う際に
    // 逐一try/catchせずに済む)。
    const JsonValue* Find(const std::string& key) const;

    static JsonValue MakeNull();
    static JsonValue MakeBool(bool value);
    static JsonValue MakeNumber(double value);
    static JsonValue MakeString(std::string value);
    static JsonValue MakeArray(std::vector<JsonValue> values);
    static JsonValue MakeObject(std::map<std::string, JsonValue> values);

   private:
    JsonType type_;
    bool boolValue_ = false;
    double numberValue_ = 0.0;
    std::string stringValue_;
    std::vector<JsonValue> arrayValue_;
    std::map<std::string, JsonValue> objectValue_;
};

// text全体を1つのJSON文書としてパースする。構文エラーの場合はJsonParseErrorを投げる。
JsonValue ParseJson(const std::string& text);

}  // namespace json
