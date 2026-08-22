// 35. シリアル通信
//
// COMポートのオープン/設定/送受信をWindows API(CreateFile/SetCommState等)で
// 実演するCLIツール。実機/仮想COMポートペアが本開発環境に無いため、
// 「probe」でオープンと設定適用の確認、「send」/「receive」で送受信APIの
// 呼び出しが正しく成功する(ハングしない)ことを確認できるようにしている
// (詳細はREADMEの「動作確認・環境上の制約」を参照)。
#include <windows.h>

#include <iostream>
#include <string>

#include "serial_port.h"

namespace {

void PrintUsage(const char* programName) {
    std::cout << "使い方:\n"
              << "  " << programName << " probe <ポート名> [ボーレート(既定9600)]\n"
              << "  " << programName << " send <ポート名> <メッセージ> [ボーレート]\n"
              << "  " << programName << " receive <ポート名> [タイムアウトms(既定1000)] [ボーレート]\n"
              << "例: " << programName << " probe COM1 115200\n";
}

const char* ParityName(uint8_t parity) {
    switch (parity) {
        case NOPARITY:
            return "None";
        case EVENPARITY:
            return "Even";
        case ODDPARITY:
            return "Odd";
        default:
            return "?";
    }
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    if (argc < 3) {
        PrintUsage(argv[0]);
        return 1;
    }

    const std::string mode = argv[1];
    const std::string portName = argv[2];

    try {
        if (mode == "probe") {
            const uint32_t baudRate = argc > 3 ? static_cast<uint32_t>(std::stoul(argv[3])) : 9600;
            serial::SerialPort port;
            serial::SerialSettings settings;
            settings.baudRate = baudRate;
            port.Open(portName, settings);
            std::cout << portName << " のオープンに成功しました。\n";

            const serial::AppliedSettings applied = port.GetAppliedSettings();
            std::cout << "適用された設定(GetCommStateで読み戻し):\n"
                      << "  ボーレート: " << applied.baudRate << "\n"
                      << "  データビット: " << static_cast<int>(applied.dataBits) << "\n"
                      << "  パリティ: " << ParityName(applied.parity) << "\n"
                      << "  ストップビット: "
                      << (applied.stopBits == ONESTOPBIT ? "1" : applied.stopBits == ONE5STOPBITS ? "1.5" : "2")
                      << "\n";
        } else if (mode == "send") {
            if (argc < 4) {
                PrintUsage(argv[0]);
                return 1;
            }
            const std::string message = argv[3];
            const uint32_t baudRate = argc > 4 ? static_cast<uint32_t>(std::stoul(argv[4])) : 9600;
            serial::SerialPort port;
            serial::SerialSettings settings;
            settings.baudRate = baudRate;
            port.Open(portName, settings);
            const size_t written = port.Write(message + "\n");
            std::cout << written << "バイトを " << portName << " へ送信しました: " << message << "\n";
        } else if (mode == "receive") {
            const uint32_t timeoutMs = argc > 3 ? static_cast<uint32_t>(std::stoul(argv[3])) : 1000;
            const uint32_t baudRate = argc > 4 ? static_cast<uint32_t>(std::stoul(argv[4])) : 9600;
            serial::SerialPort port;
            serial::SerialSettings settings;
            settings.baudRate = baudRate;
            port.Open(portName, settings);
            port.SetReadTimeout(timeoutMs);

            std::cout << portName << " から最大 " << timeoutMs << "ms 待って受信します...\n";
            const std::string received = port.Read();
            if (received.empty()) {
                std::cout << "タイムアウトしました(何も受信できませんでした)。\n";
            } else {
                std::cout << "受信(" << received.size() << "バイト): " << received << "\n";
            }
        } else {
            PrintUsage(argv[0]);
            return 1;
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
