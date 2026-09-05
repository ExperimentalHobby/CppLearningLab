// 43. HIDデータ送受信
//
// 接続中のHIDデバイスを一覧表示し、番号で選んだデバイスからInput Reportを
// 指定回数読み取ってコンソールへ表示する。
#include <iostream>
#include <limits>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "hid_device.h"

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

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::vector<hid::HidDeviceInfo> devices;
    try {
        devices = hid::EnumerateHidDevices();
    } catch (const hid::HidError& e) {
        std::cerr << "HIDデバイスの列挙に失敗しました: " << e.what() << "\n";
        return 1;
    }

    std::cout << "接続中のHIDデバイス: " << devices.size() << "件\n\n";
    for (size_t i = 0; i < devices.size(); ++i) {
        const auto& d = devices[i];
        std::cout << "[" << i << "] VID_" << std::hex << d.vendorId << " PID_" << d.productId
                   << std::dec << " " << WideToUtf8(d.product) << " (" << WideToUtf8(d.manufacturer)
                   << ")\n";
    }

    std::cout << "\nレポートを読み取るデバイス番号を入力してください(何も入力せずEnterで終了): ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) {
        return 0;
    }

    size_t selected = 0;
    try {
        // 非数値入力(std::invalid_argument)や桁あふれ(std::out_of_range)は
        // 未捕捉のまま異常終了させず、CLIとしてエラーメッセージを返して終了する。
        selected = static_cast<size_t>(std::stoul(line));
    } catch (const std::exception&) {
        std::cerr << "数値を入力してください。\n";
        return 1;
    }
    if (selected >= devices.size()) {
        std::cerr << "番号が範囲外です。\n";
        return 1;
    }

    std::cout << "レポートを5回読み取ります(デバイスからの入力を待ちます)...\n";
    try {
        const auto reports = hid::ReadReports(devices[selected].devicePath, 5);
        for (size_t i = 0; i < reports.size(); ++i) {
            std::cout << "[" << (i + 1) << "] ReportID=" << static_cast<int>(reports[i].reportId)
                       << " Data=" << hid::FormatBytes(reports[i].data) << "\n";
        }
    } catch (const hid::HidError& e) {
        std::cerr << "レポートの読み取りに失敗しました: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
