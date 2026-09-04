#include "text_file.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace {

// テスト用の一時ファイルパスを生成し、デストラクタで確実に削除するRAIIヘルパー。
class TempFilePath {
public:
    TempFilePath() {
        path_ = std::filesystem::temp_directory_path() / "simple_text_editor_test.txt";
    }
    ~TempFilePath() { std::filesystem::remove(path_); }

    const std::wstring& Path() const { return path_.native(); }

private:
    std::filesystem::path path_;
};

}  // namespace

TEST(Utf8WideConversionTest, RoundTripsAsciiText) {
    const std::string original = "Hello, World!";
    EXPECT_EQ(WideToUtf8(Utf8ToWide(original)), original);
}

// 日本語(マルチバイト文字)を含む文字列でも、UTF-8とUTF-16の変換で情報が失われない。
TEST(Utf8WideConversionTest, RoundTripsJapaneseText) {
    const std::string original = "こんにちは、世界！";
    EXPECT_EQ(WideToUtf8(Utf8ToWide(original)), original);
}

TEST(Utf8WideConversionTest, HandlesEmptyString) {
    EXPECT_EQ(Utf8ToWide(""), L"");
    EXPECT_EQ(WideToUtf8(L""), "");
}

TEST(TextFileTest, SaveThenLoadRoundTripsContent) {
    TempFilePath file;
    const std::wstring content = L"1行目\n2行目\n";

    ASSERT_TRUE(SaveTextFile(file.Path(), content));

    std::wstring loaded;
    ASSERT_TRUE(LoadTextFile(file.Path(), &loaded));
    EXPECT_EQ(loaded, content);
}

TEST(TextFileTest, LoadReturnsFalseWhenFileDoesNotExist) {
    std::wstring loaded;
    EXPECT_FALSE(LoadTextFile(L"no_such_file_12345.txt", &loaded));
}
