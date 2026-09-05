#include "usb_device.h"

#include <windows.h>

#include <cfgmgr32.h>  // MAX_DEVICE_ID_LEN
#include <setupapi.h>

#include <iterator>

#pragma comment(lib, "setupapi.lib")

namespace usb {

namespace {

std::string LastErrorMessage() {
    return "エラーコード=" + std::to_string(GetLastError());
}

// SPDRP_FRIENDLYNAME(ユーザー向けの分かりやすい名前、設定されていないデバイスも多い)を
// 優先して読み、無ければSPDRP_DEVICEDESC(デバイスクラスの標準的な説明文)で代用する。
std::wstring GetDeviceDescription(HDEVINFO devInfoSet, SP_DEVINFO_DATA& devInfoData) {
    wchar_t buffer[512]{};
    if (SetupDiGetDeviceRegistryPropertyW(devInfoSet, &devInfoData, SPDRP_FRIENDLYNAME, nullptr,
                                           reinterpret_cast<PBYTE>(buffer), sizeof(buffer), nullptr)) {
        return buffer;
    }
    if (SetupDiGetDeviceRegistryPropertyW(devInfoSet, &devInfoData, SPDRP_DEVICEDESC, nullptr,
                                           reinterpret_cast<PBYTE>(buffer), sizeof(buffer), nullptr)) {
        return buffer;
    }
    return L"(説明なし)";
}

// SPDRP_MFG(メーカー名)を取得する。多くのUSBデバイスでこのプロパティ自体が
// 設定されていないため、取得できない場合は例外にせず空文字列を返す。
std::wstring GetManufacturer(HDEVINFO devInfoSet, SP_DEVINFO_DATA& devInfoData) {
    wchar_t buffer[512]{};
    if (SetupDiGetDeviceRegistryPropertyW(devInfoSet, &devInfoData, SPDRP_MFG, nullptr,
                                           reinterpret_cast<PBYTE>(buffer), sizeof(buffer), nullptr)) {
        return buffer;
    }
    return L"";
}

}  // namespace

std::vector<UsbDeviceEntry> EnumerateUsbDevices() {
    // 第2引数(Enumerator)に"USB"を指定することで、USBバス配下に列挙されている
    // デバイス(HID/CDC/大容量記憶装置等、クラスを問わず)全てを対象にする。
    // DIGCF_ALLCLASSESと組み合わせないとデバイスクラスによる絞り込みが働いてしまう。
    const HDEVINFO devInfoSet =
        SetupDiGetClassDevsW(nullptr, L"USB", nullptr, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        throw UsbEnumerationError("SetupDiGetClassDevsWに失敗しました: " + LastErrorMessage());
    }

    std::vector<UsbDeviceEntry> devices;
    SP_DEVINFO_DATA devInfoData{};
    devInfoData.cbSize = sizeof(devInfoData);

    for (DWORD index = 0; SetupDiEnumDeviceInfo(devInfoSet, index, &devInfoData); ++index) {
        wchar_t instanceId[MAX_DEVICE_ID_LEN]{};
        if (!SetupDiGetDeviceInstanceIdW(devInfoSet, &devInfoData, instanceId,
                                         static_cast<DWORD>(std::size(instanceId)), nullptr)) {
            continue;  // インスタンスIDが取れないデバイスは読み飛ばす
        }

        UsbDeviceEntry entry;
        entry.instanceId = instanceId;
        entry.description = GetDeviceDescription(devInfoSet, devInfoData);
        entry.manufacturer = GetManufacturer(devInfoSet, devInfoData);
        devices.push_back(std::move(entry));
    }

    // SetupDiEnumDeviceInfoは列挙し尽くすとERROR_NO_MORE_ITEMSでfalseを返す仕様であり、
    // それ以外のエラーで途中終了した場合と区別するため最後にチェックする。
    const DWORD lastError = GetLastError();
    SetupDiDestroyDeviceInfoList(devInfoSet);
    if (lastError != ERROR_NO_MORE_ITEMS) {
        throw UsbEnumerationError("SetupDiEnumDeviceInfoに失敗しました: エラーコード=" +
                                   std::to_string(lastError));
    }

    return devices;
}

}  // namespace usb
