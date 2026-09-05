#include "hid_device.h"

#include <windows.h>

// hidsdi.hが内部でhidpi.hに必要な型(USAGE等)を用意するため、
// hidpi.hより先にhidsdi.hをincludeする必要がある。
#include <hidsdi.h>
#include <hidpi.h>
#include <setupapi.h>

#include <vector>

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

namespace hid {

namespace {

std::string LastErrorMessage() {
    return "エラーコード=" + std::to_string(GetLastError());
}

// SP_DEVICE_INTERFACE_DETAIL_DATA_Wは可変長構造体(DevicePathが末尾に続く)のため、
// 必要なバイト数を事前に問い合わせてから確保する。
std::wstring GetDeviceInterfacePath(HDEVINFO devInfoSet, SP_DEVICE_INTERFACE_DATA& interfaceData) {
    DWORD requiredSize = 0;
    SetupDiGetDeviceInterfaceDetailW(devInfoSet, &interfaceData, nullptr, 0, &requiredSize, nullptr);
    if (requiredSize == 0) {
        return L"";
    }

    std::vector<BYTE> buffer(requiredSize);
    auto* detail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(buffer.data());
    detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
    if (!SetupDiGetDeviceInterfaceDetailW(devInfoSet, &interfaceData, detail, requiredSize, nullptr,
                                          nullptr)) {
        return L"";
    }
    return detail->DevicePath;
}

// HIDデバイスの属性・文字列を読み取るためだけに一時的にオープンする。
// dwDesiredAccessを0(照会のみ)にすることで、マウス/キーボードのように
// OSのHIDクラスドライバに使用中のデバイスに対しても、排他制御に阻まれず
// 情報取得できるようにする(実際のレポート読み取り(ReadReports)は
// GENERIC_READが必要)。
HANDLE OpenForQuery(const std::wstring& devicePath) {
    return CreateFileW(devicePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                        0, nullptr);
}

std::wstring ReadHidString(HANDLE handle, BOOLEAN(WINAPI* getter)(HANDLE, PVOID, ULONG)) {
    wchar_t buffer[256]{};
    if (getter(handle, buffer, sizeof(buffer))) {
        return buffer;
    }
    return L"";  // 未対応のデバイスも多いため、失敗しても空文字列で継続する
}

}  // namespace

std::vector<HidDeviceInfo> EnumerateHidDevices() {
    GUID hidGuid{};
    HidD_GetHidGuid(&hidGuid);

    const HDEVINFO devInfoSet =
        SetupDiGetClassDevsW(&hidGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        throw HidError("SetupDiGetClassDevsWに失敗しました: " + LastErrorMessage());
    }

    std::vector<HidDeviceInfo> devices;
    SP_DEVICE_INTERFACE_DATA interfaceData{};
    interfaceData.cbSize = sizeof(interfaceData);

    for (DWORD index = 0;
         SetupDiEnumDeviceInterfaces(devInfoSet, nullptr, &hidGuid, index, &interfaceData); ++index) {
        const std::wstring devicePath = GetDeviceInterfacePath(devInfoSet, interfaceData);
        if (devicePath.empty()) {
            continue;
        }

        const HANDLE handle = OpenForQuery(devicePath);
        if (handle == INVALID_HANDLE_VALUE) {
            continue;  // 開けないデバイスは読み飛ばす
        }

        HidDeviceInfo info;
        info.devicePath = devicePath;

        HIDD_ATTRIBUTES attributes{};
        attributes.Size = sizeof(attributes);
        if (HidD_GetAttributes(handle, &attributes)) {
            info.vendorId = attributes.VendorID;
            info.productId = attributes.ProductID;
        }
        info.product = ReadHidString(handle, HidD_GetProductString);
        info.manufacturer = ReadHidString(handle, HidD_GetManufacturerString);

        CloseHandle(handle);
        devices.push_back(std::move(info));
    }

    SetupDiDestroyDeviceInfoList(devInfoSet);
    return devices;
}

std::vector<HidReport> ReadReports(const std::wstring& devicePath, int count) {
    const HANDLE handle = CreateFileW(devicePath.c_str(), GENERIC_READ,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0,
                                       nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        throw HidError("デバイスを開けませんでした(他プロセスが排他使用中の可能性があります): " +
                        LastErrorMessage());
    }

    // Input Reportのバイト数はデバイスごとに異なるため、HidD_GetPreparsedData+
    // HidP_GetCapsで実際のサイズを問い合わせる。取得できない場合は無難な既定値を使う。
    USHORT reportLength = 64;
    PHIDP_PREPARSED_DATA preparsedData = nullptr;
    if (HidD_GetPreparsedData(handle, &preparsedData)) {
        HIDP_CAPS caps{};
        if (HidP_GetCaps(preparsedData, &caps) == HIDP_STATUS_SUCCESS && caps.InputReportByteLength > 0) {
            reportLength = caps.InputReportByteLength;
        }
        HidD_FreePreparsedData(preparsedData);
    }

    std::vector<HidReport> reports;
    std::vector<uint8_t> buffer(reportLength);
    for (int i = 0; i < count; ++i) {
        DWORD bytesRead = 0;
        if (!ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr)) {
            const std::string message = "レポートの読み取りに失敗しました: " + LastErrorMessage();
            CloseHandle(handle);
            throw HidError(message);
        }
        const std::vector<uint8_t> received(buffer.begin(), buffer.begin() + bytesRead);
        reports.push_back(ParseReport(received));
    }

    CloseHandle(handle);
    return reports;
}

}  // namespace hid
