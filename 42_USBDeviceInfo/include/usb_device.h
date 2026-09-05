// USBデバイスの列挙。
//
// 推奨ライブラリのlibusbは、既存のUSBデバイス(マウス等)に接続するには
// WinUSBドライバへの差し替えが必要で、既存デバイスの動作を壊すリスクがある。
// そこでWindows標準のSetupAPI(setupapi.lib、Windows SDK標準)を使い、
// 接続中のUSBデバイスを読み取り専用で列挙する。
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace usb {

class UsbEnumerationError : public std::runtime_error {
   public:
    explicit UsbEnumerationError(const std::string& message) : std::runtime_error(message) {}
};

struct UsbDeviceEntry {
    // 例: "USB\VID_046D&PID_C33C\197633433932"。VID/PID/シリアル番号を含む
    // デバイスの一意な識別子(usb_device_info.h/.cppではこの文字列から
    // VID/PID・シリアル番号を抽出する)。
    std::wstring instanceId;

    // デバイスのフレンドリ名(無ければデバイスクラスの説明)。
    std::wstring description;

    // メーカー名(SPDRP_MFG)。多くのデバイスで未設定のため、その場合は空文字列。
    std::wstring manufacturer;
};

// 現在PCに接続されている(DIGCF_PRESENT)USBデバイスを列挙する。
// SetupAPI呼び出しに失敗した場合はUsbEnumerationErrorを投げる。
std::vector<UsbDeviceEntry> EnumerateUsbDevices();

}  // namespace usb
