#include "mcu_protocol.h"

#include <gtest/gtest.h>

using mcu::BuildCommandLine;
using mcu::Command;
using mcu::ParseResponse;

TEST(BuildCommandLineTest, BuildsLedOnCommand) {
    EXPECT_EQ(BuildCommandLine(Command::kLedOn), "LED_ON\n");
}

TEST(BuildCommandLineTest, BuildsLedOffCommand) {
    EXPECT_EQ(BuildCommandLine(Command::kLedOff), "LED_OFF\n");
}

TEST(BuildCommandLineTest, BuildsGetSensorCommand) {
    EXPECT_EQ(BuildCommandLine(Command::kGetSensor), "GET_SENSOR\n");
}

TEST(ParseResponseTest, ParsesOkResponse) {
    const auto result = ParseResponse("OK");

    EXPECT_TRUE(result.ok);
    EXPECT_FALSE(result.sensorValue.has_value());
    EXPECT_EQ(result.raw, "OK");
}

TEST(ParseResponseTest, ParsesErrorResponse) {
    const auto result = ParseResponse("ERROR");

    EXPECT_FALSE(result.ok);
    EXPECT_FALSE(result.sensorValue.has_value());
}

TEST(ParseResponseTest, ParsesSensorValueResponse) {
    const auto result = ParseResponse("SENSOR:123");

    EXPECT_TRUE(result.ok);
    ASSERT_TRUE(result.sensorValue.has_value());
    EXPECT_EQ(*result.sensorValue, 123);
}

TEST(ParseResponseTest, ParsesNegativeSensorValue) {
    const auto result = ParseResponse("SENSOR:-5");

    ASSERT_TRUE(result.sensorValue.has_value());
    EXPECT_EQ(*result.sensorValue, -5);
}

// "SENSOR:"接頭辞はあるが値が数値として解釈できない場合は不正な応答として扱う。
TEST(ParseResponseTest, TreatsNonNumericSensorValueAsInvalid) {
    const auto result = ParseResponse("SENSOR:abc");

    EXPECT_FALSE(result.ok);
    EXPECT_FALSE(result.sensorValue.has_value());
}

TEST(ParseResponseTest, TreatsUnknownResponseAsInvalid) {
    const auto result = ParseResponse("UNKNOWN_RESPONSE");

    EXPECT_FALSE(result.ok);
    EXPECT_FALSE(result.sensorValue.has_value());
    EXPECT_EQ(result.raw, "UNKNOWN_RESPONSE");
}

TEST(ParseResponseTest, TreatsEmptyStringAsInvalid) {
    const auto result = ParseResponse("");

    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.raw, "");
}
