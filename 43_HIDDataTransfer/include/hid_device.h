// HIDデバイスの列挙・オープン・レポート読み取り。
//
// 推奨ライブラリのhidapiは、内部的にはこのファイルと同じくWindows標準の
// HID API(hid.lib)+SetupAPI(setupapi.lib、いずれもWindows SDK標準)で
// 実装されている。本課題ではそれらを直接使い、外部ライブラリを追加しない。
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "hid_report.h"

namespace hid {

class HidError : public std::runtime_error {
   public:
    explicit HidError(const std::string& message) : std::runtime_error(message) {}
};

struct HidDeviceInfo {
    // CreateFileWにそのまま渡せるデバイスパス(例: "\\?\hid#vid_046d&pid_c33c#..."）。
    std::wstring devicePath;
    uint16_t vendorId = 0;
    uint16_t productId = 0;
    // 製品名/メーカー名。デバイスが対応していない場合は空文字列。
    std::wstring product;
    std::wstring manufacturer;
};

// 現在PCに接続されているHIDデバイスを列挙する。
// SetupAPI/HID API呼び出しに失敗した場合はHidErrorを投げる。
std::vector<HidDeviceInfo> EnumerateHidDevices();

// devicePathのデバイスをオープンし、Input Reportをcount回読み取って返す。
// オープンに失敗した場合(既に他プロセスが排他的に使用中の場合等)、または
// 読み取りに失敗した場合はHidErrorを投げる。
std::vector<HidReport> ReadReports(const std::wstring& devicePath, int count);

}  // namespace hid
