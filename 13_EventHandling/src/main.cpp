// 13. イベント処理
//
// テキストボックスに入力した内容を、ボタンを押すたびに別のラベルへ反映する
// 双方向インタラクションアプリ。Win32のイベント駆動モデルでは、Qtの
// 「シグナル・スロット」に相当する仕組みはコールバック(WndProc)への
// WM_COMMANDメッセージであり、(コントロールID, 通知コード)の組でどの操作が
// 行われたかを判別して処理を振り分ける。
#include <windows.h>

#include <iterator>

#include "reflect_format.h"

namespace {

constexpr wchar_t kWindowClassName[] = L"EventHandlingClass";
constexpr wchar_t kWindowTitle[] = L"13. Event Handling - C++ Learning Lab";

constexpr int kIdEdit = 101;
constexpr int kIdReflectButton = 102;
constexpr int kIdResultLabel = 103;
constexpr int kIdCountLabel = 104;

// 単一ウィンドウの学習用アプリなので、簡潔さのため子ウィンドウハンドルと状態を
// 匿名名前空間のグローバルとして保持する。
HWND g_hwndEdit = nullptr;
HWND g_hwndResultLabel = nullptr;
HWND g_hwndCountLabel = nullptr;
int g_reflectCount = 0;

void CreateChildControls(HWND hwnd, HINSTANCE hInstance) {
    CreateWindowExW(0, L"STATIC", L"入力:", WS_CHILD | WS_VISIBLE, 16, 16, 40, 24, hwnd, nullptr,
                     hInstance, nullptr);

    g_hwndEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_LEFT,
                                  60, 14, 220, 24, hwnd,
                                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdEdit)),
                                  hInstance, nullptr);

    CreateWindowExW(0, L"BUTTON", L"反映", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 292, 13, 80, 26,
                     hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdReflectButton)),
                     hInstance, nullptr);

    CreateWindowExW(0, L"STATIC", L"反映結果:", WS_CHILD | WS_VISIBLE, 16, 56, 70, 24, hwnd,
                     nullptr, hInstance, nullptr);

    g_hwndResultLabel =
        CreateWindowExW(WS_EX_CLIENTEDGE, L"STATIC", L"(まだ何も反映されていません)",
                         WS_CHILD | WS_VISIBLE | SS_LEFT, 90, 56, 280, 24, hwnd,
                         reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdResultLabel)), hInstance,
                         nullptr);

    g_hwndCountLabel = CreateWindowExW(0, L"STATIC", L"反映回数: 0", WS_CHILD | WS_VISIBLE, 16, 96,
                                        200, 24, hwnd,
                                        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdCountLabel)),
                                        hInstance, nullptr);

    const HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    EnumChildWindows(
        hwnd,
        [](HWND child, LPARAM font) -> BOOL {
            SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(font), TRUE);
            return TRUE;
        },
        reinterpret_cast<LPARAM>(hFont));
}

// 「反映」ボタンが押されたときの処理: テキストボックスの内容を読み取り、
// 結果ラベルへ反映し、反映回数を1増やして表示する。
// このように「あるコントロールの状態を読み、別のコントロールに書き戻す」処理を
// WM_COMMANDハンドラに集約するのがWin32でのイベント駆動プログラミングの基本形。
void OnReflectButtonClicked(HWND hwnd) {
    wchar_t buffer[256]{};
    GetWindowTextW(g_hwndEdit, buffer, static_cast<int>(std::size(buffer)));

    ++g_reflectCount;

    // 表示文字列の組み立てはHWNDに依存しないためreflect_format.cppに切り出してあり、
    // 単体テストで検証済み。ここではコントロールから読み取った値を渡すだけ。
    SetWindowTextW(g_hwndResultLabel, FormatReflectResult(buffer).c_str());
    SetWindowTextW(g_hwndCountLabel, FormatReflectCount(g_reflectCount).c_str());

    (void)hwnd;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            const HINSTANCE hInstance =
                reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            CreateChildControls(hwnd, hInstance);
            return 0;
        }
        case WM_COMMAND: {
            const int id = LOWORD(wParam);
            const int notificationCode = HIWORD(wParam);
            if (id == kIdReflectButton && notificationCode == BN_CLICKED) {
                OnReflectButtonClicked(hwnd);
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
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
    if (RegisterMainWindowClass(hInstance) == 0) {
        return 0;
    }

    const HWND hwnd = CreateWindowExW(0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 420, 200, nullptr, nullptr,
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
