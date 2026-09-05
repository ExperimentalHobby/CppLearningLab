#include "base64.h"

#include <gtest/gtest.h>

using ws::Base64Encode;

TEST(Base64EncodeTest, EncodesEmptyString) {
    EXPECT_EQ(Base64Encode(""), "");
}

// 3バイトちょうどの入力はパディング無しでエンコードされる。
TEST(Base64EncodeTest, EncodesThreeByteInputWithoutPadding) {
    EXPECT_EQ(Base64Encode("Man"), "TWFu");
}

// 入力が3の倍数バイトでない場合、'='でパディングされる(1バイト余り→"=="、2バイト余り→"=")。
TEST(Base64EncodeTest, PadsOneRemainingByteWithDoubleEquals) {
    EXPECT_EQ(Base64Encode("M"), "TQ==");
}

TEST(Base64EncodeTest, PadsTwoRemainingBytesWithSingleEquals) {
    EXPECT_EQ(Base64Encode("Ma"), "TWE=");
}

TEST(Base64EncodeTest, EncodesLongerString) {
    EXPECT_EQ(Base64Encode("Hello, World!"), "SGVsbG8sIFdvcmxkIQ==");
}
