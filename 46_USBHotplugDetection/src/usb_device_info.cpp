#include "usb_device_info.h"

#include <algorithm>
#include <cwctype>

namespace usb {

namespace {

std::wstring ToUpper(std::wstring text) {
    std::transform(text.begin(), text.end(), text.begin(),
                    [](wchar_t c) { return static_cast<wchar_t>(std::towupper(c)); });
    return text;
}

// markerの直後にある16進数4桁を読み取る。大文字小文字を区別せずmarkerを探すため、
// 検索は大文字化した文字列に対して行い、実際の値の切り出しは元の文字列から行う
// (16進数のパース自体は大文字小文字どちらでも解釈できるので元の文字列のままでよい)。
std::optional<uint16_t> FindHex4(const std::wstring& instanceId, const std::wstring& upperInstanceId,
                                  const wchar_t* marker) {
    const size_t pos = upperInstanceId.find(marker);
    if (pos == std::wstring::npos) {
        return std::nullopt;
    }
    const size_t hexStart = pos + std::char_traits<wchar_t>::length(marker);
    if (hexStart + 4 > instanceId.size()) {
        return std::nullopt;
    }
    const std::wstring hex = instanceId.substr(hexStart, 4);
    try {
        size_t consumed = 0;
        const unsigned long value = std::stoul(hex, &consumed, 16);
        if (consumed != 4) {
            return std::nullopt;  // 途中に16進数以外の文字が混じっていた
        }
        return static_cast<uint16_t>(value);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

}  // namespace

std::optional<UsbVidPid> ParseVidPid(const std::wstring& instanceId) {
    const std::wstring upper = ToUpper(instanceId);
    const auto vendorId = FindHex4(instanceId, upper, L"VID_");
    const auto productId = FindHex4(instanceId, upper, L"PID_");
    if (!vendorId || !productId) {
        return std::nullopt;
    }
    UsbVidPid result;
    result.vendorId = *vendorId;
    result.productId = *productId;
    return result;
}

std::wstring ExtractSerialNumber(const std::wstring& instanceId) {
    const size_t pos = instanceId.rfind(L'\\');
    if (pos == std::wstring::npos) {
        return instanceId;
    }
    return instanceId.substr(pos + 1);
}

}  // namespace usb
