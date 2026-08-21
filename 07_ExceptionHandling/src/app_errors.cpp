#include "app_errors.h"

#include <sstream>

AppError::AppError(const std::string& message) : std::runtime_error(message) {}

FileOpenError::FileOpenError(const std::string& path)
    : AppError("ファイルを開けませんでした: " + path), path_(path) {}

const std::string& FileOpenError::Path() const {
    return path_;
}

namespace {
std::string FormatParseError(const std::string& detail, int lineNumber) {
    std::ostringstream oss;
    if (lineNumber >= 0) {
        oss << lineNumber << "行目: " << detail;
    } else {
        oss << detail;
    }
    return oss.str();
}
}  // namespace

ParseError::ParseError(const std::string& detail, int lineNumber)
    : AppError(FormatParseError(detail, lineNumber)), lineNumber_(lineNumber) {}

int ParseError::LineNumber() const {
    return lineNumber_;
}

namespace {
std::string FormatRangeError(const std::string& key, int value, int minValue, int maxValue) {
    std::ostringstream oss;
    oss << key << "の値" << value << "は許容範囲[" << minValue << ", " << maxValue
        << "]外です";
    return oss.str();
}
}  // namespace

ValueOutOfRangeError::ValueOutOfRangeError(const std::string& key, int value, int minValue,
                                            int maxValue)
    : AppError(FormatRangeError(key, value, minValue, maxValue)), key_(key), value_(value) {}

const std::string& ValueOutOfRangeError::Key() const {
    return key_;
}

int ValueOutOfRangeError::Value() const {
    return value_;
}
