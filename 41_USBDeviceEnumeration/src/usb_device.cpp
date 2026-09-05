#include "usb_device.h"

#include <windows.h>

#include <cfgmgr32.h>  // MAX_DEVICE_ID_LEN
#include <setupapi.h>

#include <iterator>
#include <optional>

#pragma comment(lib, "setupapi.lib")

namespace usb {

namespace {

std::string LastErrorMessage() {
    return "エラーコード=" + std::to_string(GetLastError());
}

// SetupDiGetClassDevsWが返すHDEVINFOをスコープ終了時(正常終了・例外の
// どちらでも)確実にSetupDiDestroyDeviceInfoListで解放するためのRAIIガード。
class DevInfoSetGuard {
   public:
    explicit DevInfoSetGuard(HDEVINFO handle) : handle_(handle) {}
    ~DevInfoSetGuard() {
        if (handle_ != INVALID_HANDLE_VALUE) {
            SetupDiDestroyDeviceInfoList(handle_);
        }
    }
    DevInfoSetGuard(const DevInfoSetGuard&) = delete;
    DevInfoSetGuard& operator=(const DevInfoSetGuard&) = delete;

    HDEVINFO get() const { return handle_; }

   private:
    HDEVINFO handle_;
};

// 指定プロパティを可変長文字列として取得する。まずバッファ無しで呼び出して
// 必要サイズ(RequiredSize)を取得し、そのサイズで確保し直して再取得することで、
// 固定長バッファでは収まらない長い説明文字列も欠落させずに読み取る。
// プロパティ自体が存在しないデバイスではnulloptを返す。
std::optional<std::wstring> TryGetRegistryPropertyString(HDEVINFO devInfoSet,
                                                          SP_DEVINFO_DATA& devInfoData,
                                                          DWORD property) {
    DWORD requiredSize = 0;
    SetupDiGetDeviceRegistryPropertyW(devInfoSet, &devInfoData, property, nullptr, nullptr, 0,
                                       &requiredSize);
    if (requiredSize == 0) {
        return std::nullopt;
    }

    // requiredSizeはバイト数。wchar_t単位に切り上げ、念のため終端NUL分の
    // 余裕を持たせる。
    std::vector<wchar_t> buffer((requiredSize + sizeof(wchar_t) - 1) / sizeof(wchar_t) + 1, L'\0');
    if (!SetupDiGetDeviceRegistryPropertyW(
            devInfoSet, &devInfoData, property, nullptr, reinterpret_cast<PBYTE>(buffer.data()),
            static_cast<DWORD>(buffer.size() * sizeof(wchar_t)), nullptr)) {
        return std::nullopt;
    }
    return std::wstring(buffer.data());
}

// SPDRP_FRIENDLYNAME(ユーザー向けの分かりやすい名前、設定されていないデバイスも多い)を
// 優先して読み、無ければSPDRP_DEVICEDESC(デバイスクラスの標準的な説明文)で代用する。
std::wstring GetDeviceDescription(HDEVINFO devInfoSet, SP_DEVINFO_DATA& devInfoData) {
    if (auto friendlyName = TryGetRegistryPropertyString(devInfoSet, devInfoData, SPDRP_FRIENDLYNAME)) {
        return *friendlyName;
    }
    if (auto deviceDesc = TryGetRegistryPropertyString(devInfoSet, devInfoData, SPDRP_DEVICEDESC)) {
        return *deviceDesc;
    }
    return L"(説明なし)";
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
    // 以降で例外(push_backのbad_alloc等)が発生してもDevInfoSetGuardのデストラクタが
    // 確実にSetupDiDestroyDeviceInfoListを呼ぶため、ハンドルがリークしない。
    DevInfoSetGuard guard(devInfoSet);

    std::vector<UsbDeviceEntry> devices;
    SP_DEVINFO_DATA devInfoData{};
    devInfoData.cbSize = sizeof(devInfoData);

    for (DWORD index = 0; SetupDiEnumDeviceInfo(guard.get(), index, &devInfoData); ++index) {
        wchar_t instanceId[MAX_DEVICE_ID_LEN]{};
        if (!SetupDiGetDeviceInstanceIdW(guard.get(), &devInfoData, instanceId,
                                         static_cast<DWORD>(std::size(instanceId)), nullptr)) {
            continue;  // インスタンスIDが取れないデバイスは読み飛ばす
        }

        UsbDeviceEntry entry;
        entry.instanceId = instanceId;
        entry.description = GetDeviceDescription(guard.get(), devInfoData);
        devices.push_back(std::move(entry));
    }

    // SetupDiEnumDeviceInfoは列挙し尽くすとERROR_NO_MORE_ITEMSでfalseを返す仕様であり、
    // それ以外のエラーで途中終了した場合と区別するため最後にチェックする。
    // guardの解放(SetupDiDestroyDeviceInfoList呼び出し)でGetLastError()の値が
    // 上書きされる前に、ここで読み取っておく。
    const DWORD lastError = GetLastError();
    if (lastError != ERROR_NO_MORE_ITEMS) {
        throw UsbEnumerationError("SetupDiEnumDeviceInfoに失敗しました: エラーコード=" +
                                   std::to_string(lastError));
    }

    return devices;
}

}  // namespace usb
