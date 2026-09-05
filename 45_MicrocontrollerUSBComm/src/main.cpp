// 45. マイコンとのUSB通信
//
// 44番のUSB CDC通信(SerialPort)の上に、"LED_ON"/"LED_OFF"/"GET_SENSOR"という
// 単純なコマンド-応答プロトコル(mcu_protocol.h)を実装した対話式CLIツール。
//
// 注意: 本開発環境にはArduino等のUSB CDCデバイスが接続されていないため、
// 実際のコマンド-応答のやり取りは未確認(詳細はREADMEの「動作確認」を参照)。
#include <windows.h>

#include <iostream>
#include <string>

#include "mcu_protocol.h"
#include "serial_port.h"

namespace {

void PrintMenu() {
    std::cout << "\n1: LED_ON  2: LED_OFF  3: GET_SENSOR  0: 終了\n選択: ";
}

void PrintResponse(const mcu::ResponseResult& response) {
    if (response.raw.empty()) {
        std::cout << "(応答なし、タイムアウトしました)\n";
        return;
    }
    std::cout << "受信: \"" << response.raw << "\" -> ";
    if (response.sensorValue) {
        std::cout << "センサー値=" << *response.sensorValue << "\n";
    } else {
        std::cout << (response.ok ? "OK" : "ERROR(または未知の応答)") << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (argc < 2) {
        std::cout << "使い方: " << argv[0] << " <COMポート名> [ボーレート(既定9600)]\n";
        return 1;
    }
    const std::string portName = argv[1];
    const uint32_t baudRate = argc > 2 ? static_cast<uint32_t>(std::stoul(argv[2])) : 9600;

    try {
        serial::SerialPort port;
        serial::SerialSettings settings;
        settings.baudRate = baudRate;
        port.Open(portName, settings);
        port.SetReadTimeout(3000);
        std::cout << portName << " に接続しました。\n";

        for (;;) {
            PrintMenu();
            std::string choice;
            if (!std::getline(std::cin, choice) || choice == "0") {
                break;
            }

            mcu::Command command;
            if (choice == "1") {
                command = mcu::Command::kLedOn;
            } else if (choice == "2") {
                command = mcu::Command::kLedOff;
            } else if (choice == "3") {
                command = mcu::Command::kGetSensor;
            } else {
                std::cout << "不正な選択です。\n";
                continue;
            }

            port.Write(mcu::BuildCommandLine(command));
            std::string line = port.Read();
            // Read()はタイムアウトまでに届いた生バイト列を返すため、末尾の
            // 改行を取り除いてからParseResponseに渡す。
            while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
                line.pop_back();
            }
            PrintResponse(mcu::ParseResponse(line));
        }
    } catch (const serial::SerialError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
