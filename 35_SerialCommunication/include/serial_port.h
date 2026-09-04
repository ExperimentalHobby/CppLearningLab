// SerialPort: Windows API(CreateFile/SetCommState/SetCommTimeouts)による
// COMポートのRAIIラッパー。
//
// 本開発環境の制約(実機/仮想COMポートペアが接続されていない)については
// ルートREADME・本課題READMEを参照。ここではAPIの正しい使い方自体を
// 実演することに主眼を置く。
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace serial {

class SerialError : public std::runtime_error {
   public:
    explicit SerialError(const std::string& message) : std::runtime_error(message) {}
};

enum class Parity {
    kNone,
    kEven,
    kOdd,
};

struct SerialSettings {
    uint32_t baudRate = 9600;
    uint8_t dataBits = 8;
    Parity parity = Parity::kNone;
    uint8_t stopBits = 1;  // 1 or 2
};

// GetCommStateで実際に読み戻した設定(意図した設定が本当に適用されたことの確認用)。
struct AppliedSettings {
    uint32_t baudRate = 0;
    uint8_t dataBits = 0;
    uint8_t parity = 0;
    uint8_t stopBits = 0;
};

class SerialPort {
   public:
    SerialPort() = default;
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    // portName例: "COM1", "COM10"以降は"\\\\.\\COM10"形式が必要になるため
    // 内部で自動的に補う。
    void Open(const std::string& portName, const SerialSettings& settings);
    void Close();
    bool IsOpen() const { return handle_ != nullptr; }

    // 読み取りタイムアウト(1バイト間の最大待ち時間 = ReadIntervalTimeout、
    // および全体の最大待ち時間)をミリ秒単位で設定する。
    void SetReadTimeout(uint32_t totalTimeoutMs);

    // 実際にポートへ適用されている設定をGetCommStateで読み戻す。
    AppliedSettings GetAppliedSettings() const;

    // dataを書き込む。書き込んだバイト数を返す。
    size_t Write(const std::string& data);

    // 最大maxSizeバイトを読み取る。タイムアウトすると読めた分だけ返す
    // (0バイトのこともある。エラーではない)。
    std::string Read(size_t maxSize = 256);

   private:
    void* handle_ = nullptr;  // 実体はHANDLE。<windows.h>をヘッダーで強制しないため。
};

}  // namespace serial
