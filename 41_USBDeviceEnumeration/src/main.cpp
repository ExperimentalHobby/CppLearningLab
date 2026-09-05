// 41. USBデバイス列挙
//
// PCに接続されているUSBデバイスの一覧(インスタンスID+説明)をコンソールへ表示する。
// 列挙処理自体はusb_device.h/.cppに切り出してあり、42_USBDeviceInfoではこの
// EnumerateUsbDevices()の結果からVID/PID等の詳細情報を抽出する。
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "usb_device.h"

namespace {

#ifdef _WIN32
// デバイスの説明文字列は日本語(SPDRP_FRIENDLYNAME等)を含みうるため、std::wcoutの
// ロケール依存変換に頼らず、明示的にUTF-8へ変換してstd::coutで出力する。
std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return "";
    }
    const int len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr,
                                         0, nullptr, nullptr);
    if (len <= 0) {
        // 変換に失敗した場合、空文字やNUL埋め文字列をそのまま返すと表示が
        // 欠落・破損しているように見えてしまうため、エラーコード付きの
        // プレースホルダー文字列を返して失敗した事実が分かるようにする。
        return "(文字コード変換エラー: エラーコード=" + std::to_string(GetLastError()) + ")";
    }
    std::string utf8(static_cast<size_t>(len), '\0');
    const int written = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
                                             utf8.data(), len, nullptr, nullptr);
    if (written <= 0) {
        return "(文字コード変換エラー: エラーコード=" + std::to_string(GetLastError()) + ")";
    }
    return utf8;
}
#endif

}  // namespace

int main() {
#ifdef _WIN32
    // コンソールの出力コードページをUTF-8に合わせる。既定のコードページ(Shift-JIS等)の
    // ままだと、UTF-8で出力した日本語が文字化けする。
    SetConsoleOutputCP(CP_UTF8);

    try {
        const auto devices = usb::EnumerateUsbDevices();
        std::cout << "接続中のUSBデバイス: " << devices.size() << "件\n\n";
        for (const auto& device : devices) {
            std::cout << "インスタンスID: " << WideToUtf8(device.instanceId) << "\n";
            std::cout << "  説明         : " << WideToUtf8(device.description) << "\n\n";
        }
    } catch (const usb::UsbEnumerationError& e) {
        std::cerr << "USBデバイスの列挙に失敗しました: " << e.what() << "\n";
        return 1;
    }

    return 0;
#else
    // usb_device.cppはWindows標準のSetupAPIに直接依存しており、非Windows環境では
    // ビルド・実行できない。WideToUtf8()も_WIN32時のみ定義しているため、
    // ここを常時呼び出す構成にすると非Windows環境で未定義参照になってしまう。
    std::cerr << "このツールはWindows標準のSetupAPIに依存しており、Windows専用です。\n";
    return 1;
#endif
}
