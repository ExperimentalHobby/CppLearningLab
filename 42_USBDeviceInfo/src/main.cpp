// 42. デバイス情報取得
//
// 41番の列挙結果(インスタンスID+説明+メーカー名)に対し、インスタンスIDから
// VID/PID・シリアル番号を抽出して表示するCLIツール。
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "usb_device.h"
#include "usb_device_info.h"

namespace {

#ifdef _WIN32
std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return "";
    }
    const int len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr,
                                         0, nullptr, nullptr);
    std::string utf8(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), utf8.data(), len, nullptr,
                        nullptr);
    return utf8;
}
#endif

std::string FormatVidPid(const std::optional<usb::UsbVidPid>& vidPid) {
    if (!vidPid) {
        return "(取得不可)";
    }
    std::ostringstream oss;
    oss << "VID_" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << vidPid->vendorId
        << " PID_" << std::setw(4) << vidPid->productId;
    return oss.str();
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    try {
        const auto devices = usb::EnumerateUsbDevices();
        std::cout << "接続中のUSBデバイス: " << devices.size() << "件\n\n";
        for (const auto& device : devices) {
            const auto vidPid = usb::ParseVidPid(device.instanceId);
            const std::wstring serial = usb::ExtractSerialNumber(device.instanceId);

            std::cout << "説明       : " << WideToUtf8(device.description) << "\n";
            std::cout << "  VID/PID  : " << FormatVidPid(vidPid) << "\n";
            std::cout << "  シリアル : " << WideToUtf8(serial) << "\n";
            std::cout << "  メーカー : "
                       << (device.manufacturer.empty() ? "(不明)" : WideToUtf8(device.manufacturer))
                       << "\n\n";
        }
    } catch (const usb::UsbEnumerationError& e) {
        std::cerr << "USBデバイスの列挙に失敗しました: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
