// 12. 基本ウィジェット
//
// ボタン・ラベル・テキストボックス・チェックボックスといった基本ウィジェットに加え、
// メニューバー・ツールバー・ステータスバーを備えた「設定画面」風のウィンドウを作る。
// Win32には自動レイアウトマネージャ(QLayout相当)が無いため、WM_SIZEのたびに
// LayoutControls()で子コントロールの位置・サイズを手動計算する。
#include <windows.h>

#include <commctrl.h>

#include <iterator>

#pragma comment(lib, "comctl32.lib")

namespace {

constexpr wchar_t kWindowClassName[] = L"BasicWidgetsClass";
constexpr wchar_t kWindowTitle[] = L"12. Basic Widgets - C++ Learning Lab";

// 子コントロールのID。メニュー項目・ツールバーボタンのコマンドIDも兼ねる。
constexpr int kIdLabel = 101;
constexpr int kIdEdit = 102;
constexpr int kIdApplyButton = 103;
constexpr int kIdCheckbox = 104;
constexpr int kIdToolbar = 121;
constexpr int kIdStatusBar = 122;

constexpr int kIdMenuFileExit = 201;
constexpr int kIdMenuHelpAbout = 202;
constexpr int kIdToolbarNew = 211;
constexpr int kIdToolbarOpen = 212;
constexpr int kIdToolbarSave = 213;

// このサンプルは単一ウィンドウの学習用アプリなので、簡潔さを優先して
// 子ウィンドウハンドルを匿名名前空間のグローバルとして保持する
// （複数ウィンドウを扱う本格的なアプリではGWLP_USERDATA等で
//   ウィンドウごとの状態を持たせるのが望ましい）。
HWND g_hwndLabel = nullptr;
HWND g_hwndEdit = nullptr;
HWND g_hwndApplyButton = nullptr;
HWND g_hwndCheckbox = nullptr;
HWND g_hwndToolbar = nullptr;
HWND g_hwndStatusBar = nullptr;

void SetStatusText(const wchar_t* text) {
    if (g_hwndStatusBar != nullptr) {
        SendMessageW(g_hwndStatusBar, SB_SETTEXTW, 0, reinterpret_cast<LPARAM>(text));
    }
}

HMENU BuildMenuBar() {
    HMENU hMenuBar = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, kIdMenuFileExit, L"終了(&X)");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), L"ファイル(&F)");

    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, kIdMenuHelpAbout, L"バージョン情報(&A)");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hHelpMenu), L"ヘルプ(&H)");

    return hMenuBar;
}

HWND CreateToolbar(HWND hwndParent, HINSTANCE hInstance) {
    HWND hwndToolbar =
        CreateWindowExW(0, TOOLBARCLASSNAME, nullptr,
                         WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | CCS_NODIVIDER,
                         0, 0, 0, 0, hwndParent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdToolbar)),
                         hInstance, nullptr);

    SendMessageW(hwndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

    // テキストのみのツールバー(アイコン画像は使わない)なので、まず表示文字列を
    // ツールバー内部の文字列プールに登録し、その先頭インデックスを各ボタンから参照する。
    const wchar_t kButtonLabels[] = L"新規\0開く\0保存\0";
    const int baseIndex =
        static_cast<int>(SendMessageW(hwndToolbar, TB_ADDSTRINGW, 0,
                                       reinterpret_cast<LPARAM>(kButtonLabels)));

    TBBUTTON buttons[3]{};
    const int commandIds[3] = {kIdToolbarNew, kIdToolbarOpen, kIdToolbarSave};
    for (int i = 0; i < 3; ++i) {
        buttons[i].iBitmap = I_IMAGENONE;
        buttons[i].idCommand = commandIds[i];
        buttons[i].fsState = TBSTATE_ENABLED;
        buttons[i].fsStyle = BTNS_AUTOSIZE;
        buttons[i].iString = baseIndex + i;
    }
    SendMessageW(hwndToolbar, TB_ADDBUTTONSW, 3, reinterpret_cast<LPARAM>(buttons));
    SendMessageW(hwndToolbar, TB_AUTOSIZE, 0, 0);

    return hwndToolbar;
}

void CreateChildControls(HWND hwnd, HINSTANCE hInstance) {
    g_hwndLabel = CreateWindowExW(0, L"STATIC", L"名前:", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd,
                                   reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdLabel)),
                                   hInstance, nullptr);

    g_hwndEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_LEFT,
                                  0, 0, 0, 0, hwnd,
                                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdEdit)),
                                  hInstance, nullptr);

    g_hwndApplyButton =
        CreateWindowExW(0, L"BUTTON", L"適用", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0,
                         hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdApplyButton)),
                         hInstance, nullptr);

    g_hwndCheckbox = CreateWindowExW(
        0, L"BUTTON", L"通知を受け取る", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdCheckbox)), hInstance, nullptr);

    g_hwndToolbar = CreateToolbar(hwnd, hInstance);

    g_hwndStatusBar =
        CreateWindowExW(0, STATUSCLASSNAME, nullptr, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0,
                         0, 0, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusBar)),
                         hInstance, nullptr);
    SetStatusText(L"準備完了");

    // フォントを既定のUI用フォントに合わせる(未設定だと古いシステムフォントになる)。
    const HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    for (HWND child : {g_hwndLabel, g_hwndEdit, g_hwndApplyButton, g_hwndCheckbox}) {
        SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    }
}

// Win32にはQLayoutのような自動レイアウトマネージャが無いため、WM_SIZEのたびに
// 「ツールバー/ステータスバーを除いた残り領域」に4つのコントロールを縦に並べる。
void LayoutControls(HWND hwnd) {
    if (g_hwndToolbar == nullptr) {
        return;  // まだ子コントロール作成前
    }

    SendMessageW(g_hwndToolbar, TB_AUTOSIZE, 0, 0);
    RECT toolbarRect{};
    GetWindowRect(g_hwndToolbar, &toolbarRect);
    const int toolbarHeight = toolbarRect.bottom - toolbarRect.top;

    SendMessageW(g_hwndStatusBar, WM_SIZE, 0, 0);
    RECT statusRect{};
    GetWindowRect(g_hwndStatusBar, &statusRect);
    const int statusHeight = statusRect.bottom - statusRect.top;

    RECT clientRect{};
    GetClientRect(hwnd, &clientRect);
    const int contentTop = toolbarHeight;
    const int contentBottom = clientRect.bottom - statusHeight;
    const int contentWidth = clientRect.right - clientRect.left;

    constexpr int kMargin = 12;
    constexpr int kRowHeight = 24;
    constexpr int kRowSpacing = 8;
    constexpr int kLabelWidth = 60;

    int y = contentTop + kMargin;
    const int usableWidth = contentWidth - kMargin * 2;

    MoveWindow(g_hwndLabel, kMargin, y + 3, kLabelWidth, kRowHeight, TRUE);
    MoveWindow(g_hwndEdit, kMargin + kLabelWidth, y, usableWidth - kLabelWidth - 90, kRowHeight,
               TRUE);
    MoveWindow(g_hwndApplyButton, kMargin + usableWidth - 80, y, 80, kRowHeight, TRUE);
    y += kRowHeight + kRowSpacing;

    MoveWindow(g_hwndCheckbox, kMargin, y, usableWidth, kRowHeight, TRUE);
    y += kRowHeight + kRowSpacing;

    (void)contentBottom;  // 残り領域は今回未使用(将来コントロールを追加する余地として確保)
}

void OnCommand(HWND hwnd, WPARAM wParam) {
    const int id = LOWORD(wParam);
    const int notificationCode = HIWORD(wParam);

    switch (id) {
        case kIdMenuFileExit:
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
            return;
        case kIdMenuHelpAbout:
            MessageBoxW(hwnd, L"12. Basic Widgets\nC++ Learning Lab", L"バージョン情報",
                        MB_OK | MB_ICONINFORMATION);
            return;
        case kIdToolbarNew:
            SetStatusText(L"「新規」が押されました");
            return;
        case kIdToolbarOpen:
            SetStatusText(L"「開く」が押されました");
            return;
        case kIdToolbarSave:
            SetStatusText(L"「保存」が押されました");
            return;
        case kIdApplyButton:
            if (notificationCode == BN_CLICKED) {
                wchar_t buffer[256]{};
                GetWindowTextW(g_hwndEdit, buffer, static_cast<int>(std::size(buffer)));
                const bool checked = SendMessageW(g_hwndCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED;
                wchar_t status[320]{};
                wsprintfW(status, L"適用: 名前=\"%s\", 通知=%s", buffer,
                          checked ? L"ON" : L"OFF");
                SetStatusText(status);
            }
            return;
        default:
            return;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            const HINSTANCE hInstance =
                reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            CreateChildControls(hwnd, hInstance);
            SetMenu(hwnd, BuildMenuBar());
            return 0;
        }
        case WM_SIZE:
            LayoutControls(hwnd);
            return 0;
        case WM_COMMAND:
            OnCommand(hwnd, wParam);
            return 0;
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
    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES;  // ツールバー/ステータスバー等を含む
    InitCommonControlsEx(&icc);

    if (RegisterMainWindowClass(hInstance) == 0) {
        return 0;
    }

    const HWND hwnd = CreateWindowExW(0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 560, 360, nullptr, nullptr,
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
