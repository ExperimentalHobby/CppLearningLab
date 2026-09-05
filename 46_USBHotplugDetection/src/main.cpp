// 46. 抜き差し検知
//
// RegisterDeviceNotificationWでUSBデバイスインターフェースの追加/削除通知を
// 登録し、WM_DEVICECHANGEメッセージで接続(DBT_DEVICEARRIVAL)/切断
// (DBT_DEVICEREMOVECOMPLETE)を検知して、通知されたデバイスパスから
// VID/PID(usb_device_info.h、42番と同じロジック)を抽出してリストに表示する。
//
// GUID_DEVINTERFACE_USB_DEVICEの実体をこの翻訳単位でリンクするため、
// usbiodef.hより前にinitguid.hをインクルードする(Windows SDKの定番作法)。
#include <windows.h>

#include <commctrl.h>
#include <dbt.h>
#include <initguid.h>
#include <usbiodef.h>

#include <ctime>
#include <deque>
#include <iterator>
#include <string>

#include "usb_device_info.h"

#pragma comment(lib, "comctl32.lib")

namespace {

constexpr wchar_t kWindowClassName[] = L"USBHotplugDetectionClass";
constexpr wchar_t kWindowTitle[] = L"46. USB Hotplug Detection - C++ Learning Lab";

constexpr int kIdListView = 101;

HWND g_hwndList = nullptr;
HDEVNOTIFY g_deviceNotify = nullptr;

std::wstring CurrentTimeText() {
    time_t now = time(nullptr);
    tm localTime{};
    localtime_s(&localTime, &now);
    wchar_t buffer[16]{};
    wcsftime(buffer, std::size(buffer), L"%H:%M:%S", &localTime);
    return buffer;
}

void SetupListViewColumns() {
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    struct ColumnDef {
        const wchar_t* title;
        int width;
    };
    const ColumnDef columns[] = {
        {L"時刻", 80},
        {L"イベント", 80},
        {L"VID/PID", 140},
        {L"デバイスパス", 400},
    };
    for (int i = 0; i < static_cast<int>(std::size(columns)); ++i) {
        col.pszText = const_cast<wchar_t*>(columns[i].title);
        col.cx = columns[i].width;
        col.iSubItem = i;
        ListView_InsertColumn(g_hwndList, i, &col);
    }
}

// 接続/切断イベントを新しい順にリストの先頭へ追加する。
void AppendEvent(const wchar_t* eventName, const std::wstring& devicePath) {
    const auto vidPid = usb::ParseVidPid(devicePath);
    wchar_t vidPidText[32]{};
    if (vidPid) {
        wsprintfW(vidPidText, L"VID_%04X PID_%04X", vidPid->vendorId, vidPid->productId);
    } else {
        wcscpy_s(vidPidText, L"(不明)");
    }

    LVITEMW item{};
    item.mask = LVIF_TEXT;
    item.iItem = 0;
    item.iSubItem = 0;
    const std::wstring timeText = CurrentTimeText();
    item.pszText = const_cast<wchar_t*>(timeText.c_str());
    const int row = ListView_InsertItem(g_hwndList, &item);

    ListView_SetItemText(g_hwndList, row, 1, const_cast<wchar_t*>(eventName));
    ListView_SetItemText(g_hwndList, row, 2, vidPidText);
    ListView_SetItemText(g_hwndList, row, 3, const_cast<wchar_t*>(devicePath.c_str()));
}

void RegisterForDeviceNotifications(HWND hwnd) {
    DEV_BROADCAST_DEVICEINTERFACE_W filter{};
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

    // DEVICE_NOTIFY_WINDOW_HANDLE: このウィンドウのWndProcへWM_DEVICECHANGEとして
    // 通知を届ける(サービス用のDEVICE_NOTIFY_SERVICE_HANDLEは今回不要)。
    g_deviceNotify = RegisterDeviceNotificationW(hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            const HINSTANCE hInstance =
                reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            g_hwndList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                         WS_CHILD | WS_VISIBLE | LVS_REPORT, 0, 0, 0, 0, hwnd,
                                         reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdListView)),
                                         hInstance, nullptr);
            ListView_SetExtendedListViewStyle(g_hwndList, LVS_EX_FULLROWSELECT);
            SetupListViewColumns();

            RegisterForDeviceNotifications(hwnd);
            return 0;
        }
        case WM_SIZE: {
            RECT clientRect{};
            GetClientRect(hwnd, &clientRect);
            MoveWindow(g_hwndList, 0, 0, clientRect.right, clientRect.bottom, TRUE);
            return 0;
        }
        case WM_DEVICECHANGE: {
            if (wParam != DBT_DEVICEARRIVAL && wParam != DBT_DEVICEREMOVECOMPLETE) {
                break;  // 接続/切断以外の通知(設定変更等)は本課題の対象外
            }
            const auto* header = reinterpret_cast<DEV_BROADCAST_HDR*>(lParam);
            if (header == nullptr || header->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE) {
                break;
            }
            const auto* deviceInterface = reinterpret_cast<const DEV_BROADCAST_DEVICEINTERFACE_W*>(header);
            const wchar_t* eventName = (wParam == DBT_DEVICEARRIVAL) ? L"接続" : L"切断";
            AppendEvent(eventName, deviceInterface->dbcc_name);
            return TRUE;
        }
        case WM_DESTROY:
            if (g_deviceNotify != nullptr) {
                UnregisterDeviceNotification(g_deviceNotify);
            }
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

ATOM RegisterMainWindowClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kWindowClassName;
    return RegisterClassExW(&wc);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, PWSTR /*pCmdLine*/,
                     int nCmdShow) {
    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    if (RegisterMainWindowClass(hInstance) == 0) {
        return 0;
    }

    const HWND hwnd = CreateWindowExW(0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 720, 360, nullptr, nullptr,
                                       hInstance, nullptr);
    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
