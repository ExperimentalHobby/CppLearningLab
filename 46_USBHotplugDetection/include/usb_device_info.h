// USBデバイスのインスタンスIDからVID/PID・シリアル番号を抽出する純粋な文字列処理。
// SetupAPI呼び出しを一切含まないため、単体テストできる。
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace usb {

struct UsbVidPid {
    uint16_t vendorId = 0;
    uint16_t productId = 0;
};

// instanceId(例: "USB\VID_046D&PID_C33C&MI_00\..."）から
// VID(Vendor ID)/PID(Product ID)を抽出する。"VID_XXXX"/"PID_XXXX"
// (XXXXは16進数4桁、大文字小文字は問わない)が見つからない場合はstd::nulloptを返す。
std::optional<UsbVidPid> ParseVidPid(const std::wstring& instanceId);

// instanceIdの最後の'\'以降の部分を返す。単純なデバイスではシリアル番号
// (例: "197633433932")、複合デバイス(MI_xxを含む機能インターフェース)では
// バスの物理的な位置を表すロケーション文字列(例: "6&DE2EAF5&0&0002")になる。
std::wstring ExtractSerialNumber(const std::wstring& instanceId);

}  // namespace usb
