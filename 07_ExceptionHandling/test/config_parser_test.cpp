#include "config_parser.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "app_errors.h"

namespace {

// テスト用の一時ファイルを作成し、デストラクタで確実に削除するRAIIヘルパー。
class TempConfigFile {
public:
    explicit TempConfigFile(const std::string& content) {
        path_ = std::filesystem::temp_directory_path() / "exception_handling_test_config.txt";
        std::ofstream file(path_);
        file << content;
    }

    ~TempConfigFile() { std::filesystem::remove(path_); }

    std::string Path() const { return path_.string(); }

private:
    std::filesystem::path path_;
};

}  // namespace

TEST(ParseConfigFileTest, ParsesKeyValuePairs) {
    TempConfigFile file("host=localhost\nport=8080\n");

    const auto config = ParseConfigFile(file.Path());

    EXPECT_EQ(config.at("host"), "localhost");
    EXPECT_EQ(config.at("port"), "8080");
}

TEST(ParseConfigFileTest, SkipsBlankLines) {
    TempConfigFile file("host=localhost\n\nport=8080\n");

    const auto config = ParseConfigFile(file.Path());

    EXPECT_EQ(config.size(), 2u);
}

TEST(ParseConfigFileTest, ThrowsFileOpenErrorWhenFileDoesNotExist) {
    EXPECT_THROW(ParseConfigFile("no_such_file_12345.txt"), FileOpenError);
}

// '='を含まない行はParseError(行番号付き)を投げる。
TEST(ParseConfigFileTest, ThrowsParseErrorForLineWithoutEquals) {
    TempConfigFile file("host=localhost\ninvalid_line\n");

    try {
        ParseConfigFile(file.Path());
        FAIL() << "ParseErrorが投げられなかった";
    } catch (const ParseError& error) {
        EXPECT_EQ(error.LineNumber(), 2);
    }
}

TEST(ParsePositiveIntTest, ParsesValidPositiveInteger) {
    EXPECT_EQ(ParsePositiveInt("port", "8080"), 8080);
}

TEST(ParsePositiveIntTest, ThrowsParseErrorForNonNumericValue) {
    EXPECT_THROW(ParsePositiveInt("port", "abc"), ParseError);
}

TEST(ParsePositiveIntTest, ThrowsParseErrorForNonPositiveValue) {
    EXPECT_THROW(ParsePositiveInt("port", "0"), ParseError);
    EXPECT_THROW(ParsePositiveInt("port", "-1"), ParseError);
}

TEST(ValidatePortTest, AcceptsValuesWithinValidRange) {
    EXPECT_NO_THROW(ValidatePort(1));
    EXPECT_NO_THROW(ValidatePort(65535));
}

TEST(ValidatePortTest, ThrowsValueOutOfRangeErrorForValuesOutsideRange) {
    EXPECT_THROW(ValidatePort(0), ValueOutOfRangeError);
    EXPECT_THROW(ValidatePort(65536), ValueOutOfRangeError);
}
