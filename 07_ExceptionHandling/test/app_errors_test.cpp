#include "app_errors.h"

#include <gtest/gtest.h>

TEST(FileOpenErrorTest, MessageContainsPathAndPathIsAccessible) {
    const FileOpenError error("config.txt");

    EXPECT_EQ(error.Path(), "config.txt");
    EXPECT_NE(std::string(error.what()).find("config.txt"), std::string::npos);
}

TEST(ParseErrorTest, MessageIncludesLineNumberWhenProvided) {
    const ParseError error("不正な行です", 3);

    EXPECT_EQ(error.LineNumber(), 3);
    EXPECT_NE(std::string(error.what()).find("3行目"), std::string::npos);
}

// 行番号が不明(-1)な場合は、メッセージに行番号を含めない。
TEST(ParseErrorTest, MessageOmitsLineNumberWhenUnknown) {
    const ParseError error("不正な値です", -1);

    EXPECT_EQ(error.LineNumber(), -1);
    EXPECT_EQ(std::string(error.what()), "不正な値です");
}

TEST(ValueOutOfRangeErrorTest, ExposesKeyAndValue) {
    const ValueOutOfRangeError error("port", 70000, 1, 65535);

    EXPECT_EQ(error.Key(), "port");
    EXPECT_EQ(error.Value(), 70000);
}

// AppErrorの派生クラスは、AppError&(ひいてはstd::exception&)としてまとめてcatchできる。
TEST(AppErrorTest, DerivedErrorsCanBeCaughtAsAppError) {
    try {
        throw FileOpenError("missing.txt");
    } catch (const AppError& error) {
        SUCCEED();
        return;
    }
    FAIL() << "AppErrorとしてcatchされなかった";
}
