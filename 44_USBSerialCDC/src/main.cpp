// 44. USBシリアル(CDC)通信
//
// USB CDC(Communications Device Class)デバイスは、OSからは通常のCOMポートと
// して認識されるため、35_SerialCommunicationのSerialPortクラスがそのまま
// 使える。本課題ではCOMポートへ接続し、標準入力から読んだ1行を送信、
// デバイス側からの応答を受信して表示する対話ループを実装する。
//
// 注意: 本開発環境にはArduino等のUSB CDCデバイスが接続されていないため、
// 実際の送受信動作は未確認(詳細はREADMEの「動作確認」を参照)。
#include <windows.h>

#include <iostream>
#include <string>

#include "serial_port.h"

namespace {

void PrintUsage(const char* programName) {
    std::cout << "使い方: " << programName << " <COMポート名> [ボーレート(既定9600)]\n"
              << "例: " << programName << " COM3 115200\n";
}

// std::stoul()は"-1"のような負号付き文字列も受理し、符号なし整数として
// 非常に大きい値に変換してしまう(std::invalid_argumentにならない)。
// ボーレートは正の整数であるべきなので、負号・0・末尾のゴミ文字を
// 明示的に拒否する。不正な場合はstd::invalid_argumentを投げ、main()側の
// catch(std::exception&)で使い方誤りとして扱う。
uint32_t ParseBaudRate(const std::string& text) {
    if (!text.empty() && text.front() == '-') {
        throw std::invalid_argument("ボーレートに負の値は指定できません: " + text);
    }
    size_t pos = 0;
    const unsigned long value = std::stoul(text, &pos);
    if (pos != text.size()) {
        throw std::invalid_argument("ボーレートは数値で指定してください: " + text);
    }
    if (value == 0) {
        throw std::invalid_argument("ボーレートは1以上を指定してください: " + text);
    }
    return static_cast<uint32_t>(value);
}

}  // namespace

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }

    const std::string portName = argv[1];

    try {
        // USB CDCデバイスの多くは仮想的にボーレートを解釈するだけで、実際の
        // USB通信速度には影響しない(USBバス自体の速度で転送される)。それでも
        // SetCommStateへの設定自体は必要なため、通常のシリアルポートと同様に
        // 指定する。ParseBaudRate()内のstd::stoulが送出しうる例外はtry内で
        // 発生させ、下のcatchで使い方誤りとしてエラーメッセージを出せる
        // ようにする(35番と同じ構成)。
        const uint32_t baudRate = argc > 2 ? ParseBaudRate(argv[2]) : 9600;

        serial::SerialPort port;
        serial::SerialSettings settings;
        settings.baudRate = baudRate;
        port.Open(portName, settings);
        port.SetReadTimeout(3000);
        std::cout << portName << " に接続しました。文字列を入力してEnterで送信します"
                  << "(空行で終了)。\n";

        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            if (line.empty()) {
                break;
            }
            port.Write(line + "\n");

            const std::string received = port.Read();
            if (received.empty()) {
                std::cout << "(応答なし、タイムアウトしました)\n";
            } else {
                std::cout << "受信: " << received << "\n";
            }
        }
    } catch (const serial::SerialError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
