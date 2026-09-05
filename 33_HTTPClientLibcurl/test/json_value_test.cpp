#include "json_value.h"

#include <gtest/gtest.h>

using json::JsonParseError;
using json::JsonValue;
using json::ParseJson;

TEST(ParseJsonTest, ParsesNull) {
    EXPECT_TRUE(ParseJson("null").IsNull());
}

TEST(ParseJsonTest, ParsesBool) {
    EXPECT_TRUE(ParseJson("true").AsBool());
    EXPECT_FALSE(ParseJson("false").AsBool());
}

TEST(ParseJsonTest, ParsesInteger) {
    EXPECT_DOUBLE_EQ(ParseJson("42").AsNumber(), 42.0);
}

TEST(ParseJsonTest, ParsesNegativeAndFractionalNumber) {
    EXPECT_DOUBLE_EQ(ParseJson("-3.5").AsNumber(), -3.5);
}

TEST(ParseJsonTest, ParsesNumberWithExponent) {
    EXPECT_DOUBLE_EQ(ParseJson("1.5e2").AsNumber(), 150.0);
}

TEST(ParseJsonTest, ParsesString) {
    EXPECT_EQ(ParseJson("\"hello\"").AsString(), "hello");
}

// 主要なエスケープシーケンス(改行・タブ・引用符・バックスラッシュ)を解釈する。
TEST(ParseJsonTest, ParsesEscapeSequencesInString) {
    EXPECT_EQ(ParseJson("\"a\\nb\\t\\\"c\\\\d\"").AsString(), "a\nb\t\"c\\d");
}

// \uXXXXエスケープをUTF-8にデコードする(このテストの範囲は日本語などの3バイト文字)。
TEST(ParseJsonTest, ParsesUnicodeEscape) {
    EXPECT_EQ(ParseJson("\"\\u3042\"").AsString(), "あ");
}

TEST(ParseJsonTest, ParsesEmptyArray) {
    EXPECT_TRUE(ParseJson("[]").AsArray().empty());
}

TEST(ParseJsonTest, ParsesArrayOfNumbers) {
    const auto array = ParseJson("[1, 2, 3]").AsArray();

    ASSERT_EQ(array.size(), 3u);
    EXPECT_DOUBLE_EQ(array[0].AsNumber(), 1.0);
    EXPECT_DOUBLE_EQ(array[2].AsNumber(), 3.0);
}

TEST(ParseJsonTest, ParsesObjectAndFindsFieldsByKey) {
    const auto obj = ParseJson(R"({"name": "Alice", "age": 30})");

    ASSERT_TRUE(obj.IsObject());
    const auto* name = obj.Find("name");
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->AsString(), "Alice");
    const auto* age = obj.Find("age");
    ASSERT_NE(age, nullptr);
    EXPECT_DOUBLE_EQ(age->AsNumber(), 30.0);
}

// 存在しないキーはnullptrを返す(例外を投げない安全なアクセサ)。
TEST(ParseJsonTest, FindReturnsNullptrForMissingKey) {
    const auto obj = ParseJson(R"({"name": "Alice"})");

    EXPECT_EQ(obj.Find("nonexistent"), nullptr);
}

TEST(ParseJsonTest, ParsesNestedStructures) {
    const auto obj = ParseJson(R"({"items": [{"id": 1}, {"id": 2}]})");

    const auto* items = obj.Find("items");
    ASSERT_NE(items, nullptr);
    ASSERT_EQ(items->AsArray().size(), 2u);
    EXPECT_DOUBLE_EQ(items->AsArray()[1].Find("id")->AsNumber(), 2.0);
}

TEST(ParseJsonTest, ThrowsOnSyntaxError) {
    EXPECT_THROW(ParseJson("{invalid}"), JsonParseError);
}

TEST(ParseJsonTest, ThrowsOnTrailingGarbage) {
    EXPECT_THROW(ParseJson("42 extra"), JsonParseError);
}

// 型が一致しないアクセサ呼び出しは例外を投げる。
TEST(JsonValueTest, AsNumberThrowsWhenNotANumber) {
    const JsonValue value = ParseJson("\"text\"");

    EXPECT_THROW(value.AsNumber(), JsonParseError);
}

TEST(JsonValueFactoryTest, MakeFunctionsBuildExpectedTypes) {
    EXPECT_TRUE(JsonValue::MakeNull().IsNull());
    EXPECT_TRUE(JsonValue::MakeBool(true).AsBool());
    EXPECT_DOUBLE_EQ(JsonValue::MakeNumber(1.25).AsNumber(), 1.25);
    EXPECT_EQ(JsonValue::MakeString("abc").AsString(), "abc");
    EXPECT_TRUE(JsonValue::MakeArray({}).IsArray());
    EXPECT_TRUE(JsonValue::MakeObject({}).IsObject());
}
