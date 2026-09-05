#include "serial_port.h"

#include <windows.h>

namespace serial {

namespace {

std::string LastErrorMessage() {
    const DWORD code = GetLastError();
    LPWSTR buffer = nullptr;
    const DWORD length = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
        code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

    std::string message = "エラーコード=" + std::to_string(code);
    if (length > 0 && buffer != nullptr) {
        const int utf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, static_cast<int>(length), nullptr, 0,
                                                 nullptr, nullptr);
        std::string text(static_cast<size_t>(utf8Len), '\0');
        WideCharToMultiByte(CP_UTF8, 0, buffer, static_cast<int>(length), text.data(), utf8Len, nullptr,
                             nullptr);
        LocalFree(buffer);
        // FormatMessageの末尾には改行が付くことが多いので取り除く。
        while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
            text.pop_back();
        }
        message += " (" + text + ")";
    }
    return message;
}

std::wstring Utf8ToWide(const std::string& utf8) {
    const int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), wide.data(), len);
    return wide;
}

BYTE ToWinParity(Parity parity) {
    switch (parity) {
        case Parity::kNone:
            return NOPARITY;
        case Parity::kEven:
            return EVENPARITY;
        case Parity::kOdd:
            return ODDPARITY;
    }
    return NOPARITY;
}

}  // namespace

SerialPort::~SerialPort() { Close(); }

void SerialPort::Open(const std::string& portName, const SerialSettings& settings) {
    Close();

    // "\\\\.\\COM1"形式のデバイスパスにすると、COM10以降(2桁以上の番号)でも
    // 問題なく開ける(素の"COM1"形式は1桁の番号でしか正しく動作しないことがある)。
    // 呼び出し側が既にこの"\\.\"形式で渡してきた場合にここで無条件に前置すると
    // "\\.\\.\COM10"のように二重になり必ずオープンに失敗するため、まだ
    // 前置されていない場合のみ付与する。
    const std::string kDevicePrefix = "\\\\.\\";
    const std::string fullPortName = (portName.compare(0, kDevicePrefix.size(), kDevicePrefix) == 0)
                                          ? portName
                                          : kDevicePrefix + portName;
    const std::wstring devicePath = Utf8ToWide(fullPortName);

    HANDLE handle = CreateFileW(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                                 0, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        throw SerialError("ポートを開けませんでした(" + portName + "): " + LastErrorMessage());
    }

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(handle, &dcb)) {
        const std::string message = "GetCommStateに失敗しました: " + LastErrorMessage();
        CloseHandle(handle);
        throw SerialError(message);
    }

    dcb.BaudRate = settings.baudRate;
    dcb.ByteSize = settings.dataBits;
    dcb.Parity = ToWinParity(settings.parity);
    if (settings.stopBits == 1) {
        dcb.StopBits = ONESTOPBIT;
    } else if (settings.stopBits == 2) {
        dcb.StopBits = TWOSTOPBITS;
    } else {
        CloseHandle(handle);
        throw SerialError("無効なストップビット数です(1または2を指定してください): " +
                          std::to_string(settings.stopBits));
    }
    dcb.fBinary = TRUE;  // Windowsのシリアル通信はバイナリモード必須(テキストモード非対応)。
    dcb.fParity = settings.parity != Parity::kNone;  // パリティ無し以外を選んだ場合のみ検査を有効化。

    if (!SetCommState(handle, &dcb)) {
        const std::string message = "SetCommStateに失敗しました: " + LastErrorMessage();
        CloseHandle(handle);
        throw SerialError(message);
    }

    handle_ = handle;

    // 既定では無限待ちになりうるため、Open直後に妥当なタイムアウトを設定しておく。
    SetReadTimeout(1000);
}

void SerialPort::Close() {
    if (handle_ != nullptr) {
        CloseHandle(static_cast<HANDLE>(handle_));
        handle_ = nullptr;
    }
}

void SerialPort::SetReadTimeout(uint32_t totalTimeoutMs) {
    if (handle_ == nullptr) {
        throw SerialError("ポートが開かれていません");
    }
    COMMTIMEOUTS timeouts{};
    // ReadIntervalTimeout=MAXDWORD + ReadTotalTimeoutConstant>0 という組み合わせは
    // 「1バイトでも受信済みならすぐ返す、何も無ければConstantミリ秒で打ち切る」
    // という挙動になる(Win32ドキュメント記載の特殊な組み合わせ)。
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = totalTimeoutMs;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 1000;

    if (!SetCommTimeouts(static_cast<HANDLE>(handle_), &timeouts)) {
        throw SerialError("SetCommTimeoutsに失敗しました: " + LastErrorMessage());
    }
}

AppliedSettings SerialPort::GetAppliedSettings() const {
    if (handle_ == nullptr) {
        throw SerialError("ポートが開かれていません");
    }
    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(static_cast<HANDLE>(handle_), &dcb)) {
        throw SerialError("GetCommStateに失敗しました: " + LastErrorMessage());
    }
    AppliedSettings applied;
    applied.baudRate = dcb.BaudRate;
    applied.dataBits = dcb.ByteSize;
    applied.parity = dcb.Parity;
    applied.stopBits = dcb.StopBits;
    return applied;
}

size_t SerialPort::Write(const std::string& data) {
    if (handle_ == nullptr) {
        throw SerialError("ポートが開かれていません");
    }
    // WriteFileへ渡すサイズはDWORD(32bit)のため、data.size()(64bit環境では
    // size_t)がMAXDWORDを超えると静かに切り詰められ、戻り値や実際の送信
    // 内容が呼び出し側の意図と一致しなくなる。あらかじめ範囲を検証する。
    if (data.size() > MAXDWORD) {
        throw SerialError("書き込みデータが大きすぎます(DWORDの範囲を超えています): " +
                          std::to_string(data.size()) + " bytes");
    }
    DWORD written = 0;
    if (!WriteFile(static_cast<HANDLE>(handle_), data.data(), static_cast<DWORD>(data.size()), &written,
                    nullptr)) {
        throw SerialError("書き込みに失敗しました: " + LastErrorMessage());
    }
    return static_cast<size_t>(written);
}

std::string SerialPort::Read(size_t maxSize) {
    if (handle_ == nullptr) {
        throw SerialError("ポートが開かれていません");
    }
    if (maxSize == 0) {
        return "";
    }
    // ReadFileへ渡すサイズはDWORD(32bit)のため、maxSizeがMAXDWORDを超えると
    // バッファ確保サイズとWinAPI呼び出しサイズが不整合になる。
    if (maxSize > MAXDWORD) {
        throw SerialError("読み取りサイズが大きすぎます(DWORDの範囲を超えています): " +
                          std::to_string(maxSize) + " bytes");
    }
    std::string buffer(maxSize, '\0');
    DWORD readBytes = 0;
    if (!ReadFile(static_cast<HANDLE>(handle_), buffer.data(), static_cast<DWORD>(maxSize), &readBytes,
                   nullptr)) {
        throw SerialError("読み取りに失敗しました: " + LastErrorMessage());
    }
    buffer.resize(readBytes);  // タイムアウト時は0バイトになる(エラーではない)
    return buffer;
}

}  // namespace serial
