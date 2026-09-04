#include "json_value.h"

#include <cctype>
#include <cstdlib>

namespace json {

bool JsonValue::AsBool() const {
    if (type_ != JsonType::kBool) {
        throw JsonParseError("bool型ではありません");
    }
    return boolValue_;
}

double JsonValue::AsNumber() const {
    if (type_ != JsonType::kNumber) {
        throw JsonParseError("数値型ではありません");
    }
    return numberValue_;
}

const std::string& JsonValue::AsString() const {
    if (type_ != JsonType::kString) {
        throw JsonParseError("文字列型ではありません");
    }
    return stringValue_;
}

const std::vector<JsonValue>& JsonValue::AsArray() const {
    if (type_ != JsonType::kArray) {
        throw JsonParseError("配列型ではありません");
    }
    return arrayValue_;
}

const JsonValue* JsonValue::Find(const std::string& key) const {
    if (type_ != JsonType::kObject) {
        return nullptr;
    }
    const auto it = objectValue_.find(key);
    return it != objectValue_.end() ? &it->second : nullptr;
}

JsonValue JsonValue::MakeNull() { return JsonValue(); }

JsonValue JsonValue::MakeBool(bool value) {
    JsonValue v;
    v.type_ = JsonType::kBool;
    v.boolValue_ = value;
    return v;
}

JsonValue JsonValue::MakeNumber(double value) {
    JsonValue v;
    v.type_ = JsonType::kNumber;
    v.numberValue_ = value;
    return v;
}

JsonValue JsonValue::MakeString(std::string value) {
    JsonValue v;
    v.type_ = JsonType::kString;
    v.stringValue_ = std::move(value);
    return v;
}

JsonValue JsonValue::MakeArray(std::vector<JsonValue> values) {
    JsonValue v;
    v.type_ = JsonType::kArray;
    v.arrayValue_ = std::move(values);
    return v;
}

JsonValue JsonValue::MakeObject(std::map<std::string, JsonValue> values) {
    JsonValue v;
    v.type_ = JsonType::kObject;
    v.objectValue_ = std::move(values);
    return v;
}

namespace {

// 再帰下降法によるJSONパーサー。textはUTF-8前提。
class Parser {
   public:
    explicit Parser(const std::string& text) : text_(text) {}

    JsonValue ParseDocument() {
        SkipWhitespace();
        JsonValue value = ParseValue();
        SkipWhitespace();
        if (pos_ != text_.size()) {
            throw JsonParseError("JSON文書の末尾に余分な文字があります(位置=" + std::to_string(pos_) + ")");
        }
        return value;
    }

   private:
    const std::string& text_;
    size_t pos_ = 0;

    char Peek() const {
        if (pos_ >= text_.size()) {
            throw JsonParseError("予期しない入力の終端です");
        }
        return text_[pos_];
    }

    char Next() {
        if (pos_ >= text_.size()) {
            throw JsonParseError("予期しない入力の終端です");
        }
        return text_[pos_++];
    }

    void Expect(char expected) {
        if (pos_ >= text_.size() || text_[pos_] != expected) {
            throw JsonParseError(std::string("'") + expected + "'が期待されましたが見つかりません(位置=" +
                                  std::to_string(pos_) + ")");
        }
        ++pos_;
    }

    void SkipWhitespace() {
        while (pos_ < text_.size() &&
               (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\n' || text_[pos_] == '\r')) {
            ++pos_;
        }
    }

    bool StartsWith(const char* literal) const {
        const size_t len = std::char_traits<char>::length(literal);
        return text_.compare(pos_, len, literal) == 0;
    }

    JsonValue ParseValue() {
        SkipWhitespace();
        const char c = Peek();
        if (c == '{') return ParseObject();
        if (c == '[') return ParseArray();
        if (c == '"') return JsonValue::MakeString(ParseRawString());
        if (c == 't' || c == 'f') return ParseBool();
        if (c == 'n') return ParseNull();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return ParseNumber();
        throw JsonParseError("不正な値です(位置=" + std::to_string(pos_) + ")");
    }

    JsonValue ParseObject() {
        Expect('{');
        std::map<std::string, JsonValue> result;
        SkipWhitespace();
        if (pos_ < text_.size() && Peek() == '}') {
            Next();
            return JsonValue::MakeObject(std::move(result));
        }
        for (;;) {
            SkipWhitespace();
            const std::string key = ParseRawString();
            SkipWhitespace();
            Expect(':');
            result.emplace(key, ParseValue());
            SkipWhitespace();
            const char c = Next();
            if (c == ',') continue;
            if (c == '}') break;
            throw JsonParseError("オブジェクトの区切り文字が不正です(位置=" + std::to_string(pos_) + ")");
        }
        return JsonValue::MakeObject(std::move(result));
    }

    JsonValue ParseArray() {
        Expect('[');
        std::vector<JsonValue> result;
        SkipWhitespace();
        if (pos_ < text_.size() && Peek() == ']') {
            Next();
            return JsonValue::MakeArray(std::move(result));
        }
        for (;;) {
            result.push_back(ParseValue());
            SkipWhitespace();
            const char c = Next();
            if (c == ',') continue;
            if (c == ']') break;
            throw JsonParseError("配列の区切り文字が不正です(位置=" + std::to_string(pos_) + ")");
        }
        return JsonValue::MakeArray(std::move(result));
    }

    std::string ParseRawString() {
        Expect('"');
        std::string result;
        for (;;) {
            if (pos_ >= text_.size()) {
                throw JsonParseError("文字列が閉じられていません");
            }
            const char c = Next();
            if (c == '"') {
                break;
            }
            if (c == '\\') {
                const char escaped = Next();
                switch (escaped) {
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case '/':
                        result += '/';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'u': {
                        // \uXXXX。サロゲートペアや非ASCII全般は本課題のスコープ外とし、
                        // ここではU+0000-U+FFFFをそのままUTF-8の1〜3バイト列に変換する
                        // 簡易実装に留める(サロゲートペア(絵文字等)は非対応)。
                        if (pos_ + 4 > text_.size()) {
                            throw JsonParseError("不正な\\uエスケープです");
                        }
                        const unsigned int codePoint = static_cast<unsigned int>(
                            std::stoul(text_.substr(pos_, 4), nullptr, 16));
                        pos_ += 4;
                        if (codePoint <= 0x7F) {
                            result += static_cast<char>(codePoint);
                        } else if (codePoint <= 0x7FF) {
                            result += static_cast<char>(0xC0 | (codePoint >> 6));
                            result += static_cast<char>(0x80 | (codePoint & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | (codePoint >> 12));
                            result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (codePoint & 0x3F));
                        }
                        break;
                    }
                    default:
                        throw JsonParseError("不明なエスケープシーケンスです");
                }
            } else {
                result += c;
            }
        }
        return result;
    }

    JsonValue ParseNumber() {
        const size_t start = pos_;
        if (Peek() == '-') {
            ++pos_;
        }
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
            ++pos_;
        }
        if (pos_ < text_.size() && text_[pos_] == '.') {
            ++pos_;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                ++pos_;
            }
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) {
                ++pos_;
            }
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                ++pos_;
            }
        }
        const std::string numberText = text_.substr(start, pos_ - start);
        return JsonValue::MakeNumber(std::strtod(numberText.c_str(), nullptr));
    }

    JsonValue ParseBool() {
        if (StartsWith("true")) {
            pos_ += 4;
            return JsonValue::MakeBool(true);
        }
        if (StartsWith("false")) {
            pos_ += 5;
            return JsonValue::MakeBool(false);
        }
        throw JsonParseError("不正なリテラルです(位置=" + std::to_string(pos_) + ")");
    }

    JsonValue ParseNull() {
        if (StartsWith("null")) {
            pos_ += 4;
            return JsonValue::MakeNull();
        }
        throw JsonParseError("不正なリテラルです(位置=" + std::to_string(pos_) + ")");
    }
};

}  // namespace

JsonValue ParseJson(const std::string& text) {
    Parser parser(text);
    return parser.ParseDocument();
}

}  // namespace json
