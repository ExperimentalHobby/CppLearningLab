// 18. マルチスレッドGUI
//
// 「重い処理」(ダミーのループ)をバックグラウンドのstd::threadで実行し、
// 進捗をプログレスバーに反映しつつ、UIスレッドはブロックしない(操作可能なまま)
// ことを実演する。
//
// Win32(に限らずほとんどのGUIツールキット)は、UIスレッド以外のスレッドから
// 直接ウィジェットを操作することを許していない。そこでワーカースレッドは
// ウィンドウを直接触らず、PostMessageで「進捗が変わった」「終わった」という
// 通知だけをUIスレッドに送り、実際のプログレスバー更新はUIスレッド上の
// WndProcが行う。これがスレッド間通信の基本形。
#include <windows.h>

#include <commctrl.h>

#include <atomic>
#include <thread>

#pragma comment(lib, "comctl32.lib")

namespace {

constexpr wchar_t kWindowClassName[] = L"MultiThreadedGUIClass";
constexpr wchar_t kWindowTitle[] = L"18. Multi-Threaded GUI - C++ Learning Lab";

constexpr int kIdProgressBar = 101;
constexpr int kIdStartButton = 102;
constexpr int kIdCancelButton = 103;
constexpr int kIdStatusLabel = 104;

// WM_APP以降はアプリケーション独自のメッセージとして自由に使える範囲。
constexpr UINT kWmProgress = WM_APP + 1;  // wParam: 進捗(0-100)
constexpr UINT kWmDone = WM_APP + 2;      // wParam: 1ならキャンセルされた, 0なら完了

constexpr int kTotalSteps = 100;
constexpr int kStepDelayMs = 20;  // 1ステップあたりの「重い処理」を模したウェイト

HWND g_hwndProgress = nullptr;
HWND g_hwndStartButton = nullptr;
HWND g_hwndCancelButton = nullptr;
HWND g_hwndStatusLabel = nullptr;

std::thread g_workerThread;
std::atomic<bool> g_cancelRequested{false};

void SetStatusText(const wchar_t* text) {
    SetWindowTextW(g_hwndStatusLabel, text);
}

// ワーカースレッドで実行される関数。ウィンドウは直接操作せず、PostMessageのみで
// UIスレッドに進捗・完了を通知する。
void WorkerThreadProc(HWND hwnd) {
    bool cancelled = false;
    for (int step = 0; step <= kTotalSteps; ++step) {
        if (g_cancelRequested.load()) {
            cancelled = true;
            break;
        }
        Sleep(kStepDelayMs);  // ダミーの「重い処理」
        PostMessageW(hwnd, kWmProgress, static_cast<WPARAM>(step), 0);
    }
    PostMessageW(hwnd, kWmDone, cancelled ? 1 : 0, 0);
}

void OnStart(HWND hwnd) {
    if (g_workerThread.joinable()) {
        return;  // 既に実行中
    }
    g_cancelRequested = false;
    SendMessageW(g_hwndProgress, PBM_SETPOS, 0, 0);
    EnableWindow(g_hwndStartButton, FALSE);
    EnableWindow(g_hwndCancelButton, TRUE);
    SetStatusText(L"実行中...");
    g_workerThread = std::thread(WorkerThreadProc, hwnd);
}

void OnCancel() {
    g_cancelRequested = true;
}

void OnProgress(WPARAM wParam) {
    SendMessageW(g_hwndProgress, PBM_SETPOS, wParam, 0);
}

void OnDone(WPARAM wParam) {
    if (g_workerThread.joinable()) {
        g_workerThread.join();
    }
    EnableWindow(g_hwndStartButton, TRUE);
    EnableWindow(g_hwndCancelButton, FALSE);
    SetStatusText(wParam != 0 ? L"キャンセルされました" : L"完了しました");
}

void CreateChildControls(HWND hwnd, HINSTANCE hInstance) {
    g_hwndProgress =
        CreateWindowExW(0, PROGRESS_CLASSW, nullptr, WS_CHILD | WS_VISIBLE, 16, 16, 360, 24, hwnd,
                         reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdProgressBar)), hInstance,
                         nullptr);
    SendMessageW(g_hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, kTotalSteps));
    SendMessageW(g_hwndProgress, PBM_SETSTEP, 1, 0);

    g_hwndStartButton =
        CreateWindowExW(0, L"BUTTON", L"開始", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 16, 56, 100,
                         28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStartButton)),
                         hInstance, nullptr);

    g_hwndCancelButton = CreateWindowExW(
        0, L"BUTTON", L"キャンセル", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED, 124, 56,
        100, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdCancelButton)), hInstance,
        nullptr);

    g_hwndStatusLabel =
        CreateWindowExW(0, L"STATIC", L"待機中", WS_CHILD | WS_VISIBLE, 16, 96, 360, 24, hwnd,
                         reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusLabel)), hInstance,
                         nullptr);

    const HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    for (HWND child : {g_hwndStartButton, g_hwndCancelButton, g_hwndStatusLabel}) {
        SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    }
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
            if (id == kIdStartButton) {
                OnStart(hwnd);
            } else if (id == kIdCancelButton) {
                OnCancel();
            }
            return 0;
        }
        case kWmProgress:
            OnProgress(wParam);
            return 0;
        case kWmDone:
            OnDone(wParam);
            return 0;
        case WM_DESTROY:
            // ウィンドウを閉じるときにワーカースレッドが残っていると、joinしない
            // まま終了してクラッシュしうる(std::threadは破棄時にjoinableだと
            // std::terminateを呼ぶ)。キャンセルを要求してから必ずjoinする。
            g_cancelRequested = true;
            if (g_workerThread.joinable()) {
                g_workerThread.join();
            }
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
    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icc);

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
