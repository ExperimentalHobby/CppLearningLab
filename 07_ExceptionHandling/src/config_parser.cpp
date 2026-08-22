#include "config_parser.h"

#include <fstream>

#include "app_errors.h"
#include "scoped_trace.h"

std::map<std::string, std::string> ParseConfigFile(const std::string& path) {
    ScopedTrace trace("ParseConfigFile(" + path + ")");

    // std::ifstreamはRAIIオブジェクトなので、この関数の途中で例外が発生して
    // スコープを抜けても、デストラクタで自動的にファイルがクローズされる。
    std::ifstream file(path);
    if (!file) {
        throw FileOpenError(path);
    }

    std::map<std::string, std::string> config;
    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (line.empty()) {
            continue;
        }

        const auto equalsPos = line.find('=');
        if (equalsPos == std::string::npos) {
            throw ParseError("'='を含まない不正な行です: \"" + line + "\"", lineNumber);
        }

        const std::string key = line.substr(0, equalsPos);
        const std::string value = line.substr(equalsPos + 1);
        config[key] = value;
    }

    return config;
}

int ParsePositiveInt(const std::string& key, const std::string& value) {
    try {
        const int result = std::stoi(value);
        if (result <= 0) {
            throw ParseError(key + "の値は正の整数である必要があります: " + value, -1);
        }
        return result;
    } catch (const std::invalid_argument&) {
        // 標準例外(std::invalid_argument)を、呼び出し側にとって意味のある
        // 独自例外(ParseError)に変換して投げ直す。
        throw ParseError(key + "を数値に変換できません: \"" + value + "\"", -1);
    } catch (const std::out_of_range&) {
        throw ParseError(key + "の値が大きすぎます: \"" + value + "\"", -1);
    }
}

void ValidatePort(int port) {
    constexpr int kMinPort = 1;
    constexpr int kMaxPort = 65535;
    if (port < kMinPort || port > kMaxPort) {
        throw ValueOutOfRangeError("port", port, kMinPort, kMaxPort);
    }
}
