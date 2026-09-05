#include "mcu_protocol.h"

#include <stdexcept>
#include <string>

namespace mcu {

std::string BuildCommandLine(Command command) {
    switch (command) {
        case Command::kLedOn:
            return "LED_ON\n";
        case Command::kLedOff:
            return "LED_OFF\n";
        case Command::kGetSensor:
            return "GET_SENSOR\n";
    }
    // enum class Commandの全列挙子はswitchで網羅済みのため、ここに到達するのは
    // 未定義の値がstatic_cast等で渡された場合のみ。空文字列を返すと呼び出し側が
    // 気づかずそのまま送信してしまうため、原因を明示するために例外にする。
    throw std::invalid_argument("不明なCommand値です: " +
                                std::to_string(static_cast<int>(command)));
}

ResponseResult ParseResponse(const std::string& line) {
    ResponseResult result;
    result.raw = line;

    if (line == "OK") {
        result.ok = true;
        return result;
    }
    if (line == "ERROR") {
        result.ok = false;
        return result;
    }

    constexpr const char* kSensorPrefix = "SENSOR:";
    constexpr size_t kPrefixLength = 7;  // std::char_traits<char>::length(kSensorPrefix)
    if (line.compare(0, kPrefixLength, kSensorPrefix) == 0) {
        const std::string valueText = line.substr(kPrefixLength);
        try {
            size_t consumed = 0;
            const int value = std::stoi(valueText, &consumed);
            if (consumed == valueText.size() && !valueText.empty()) {
                result.ok = true;
                result.sensorValue = value;
            }
        } catch (const std::exception&) {
            // 数値に変換できない場合はok=falseのまま(不正な応答として扱う)。
        }
        return result;
    }

    // 未知の応答形式。ok=false・sensorValue=nulloptのまま返す。
    return result;
}

}  // namespace mcu
