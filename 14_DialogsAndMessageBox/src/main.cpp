// 14. ダイアログ
//
// メッセージボックス(情報/警告/エラー/確認)と、自作の入力ダイアログ(モーダル/モードレス)
// を実演する。
//
// 入力ダイアログは(.rcリソース+DialogBoxParamではなく)他の課題と同じ
// CreateWindowExWベースのポップアップウィンドウとして実装している。これにより
// 「モーダル/モードレスの違いは、結局のところメッセージループの回し方の違いである」
// ことがそのままコードに表れる。
//
// - モーダル: ShowModalInputDialog自身がネストしたメッセージループを回し、
//   ダイアログが閉じるまで関数から戻らない(呼び出し元=メインウィンドウの操作を
//   ブロックする)。オーナーウィンドウはEnableWindow(FALSE)で無効化する。
// - モードレス: ウィンドウを作って即座に関数から戻る。メインの
//   メッセージループがそのまま両方のウィンドウのメッセージを処理し続けるため、
//   メインウィンドウは操作可能なまま。
//
// どちらの場合も、入力結果をメインウィンドウへ安全に伝えるため、ダイアログが
// 閉じるタイミングでWM_COPYDATAによりデータをコピーして渡す
// (モーダル/モードレスの実装を統一でき、モードレス時にありがちな
//   「呼び出し元の関数は既に返ってしまっている」というライフタイム問題も避けられる)。
#include <windows.h>

#include <iterator>

namespace {

constexpr wchar_t kWindowClassName[] = L"DialogsAndMessageBoxClass";
constexpr wchar_t kWindowTitle[] = L"14. Dialogs and MessageBox - C++ Learning Lab";
constexpr wchar_t kInputDialogClassName[] = L"InputDialogClass";

constexpr int kIdConfirmButton = 101;
constexpr int kIdInfoButton = 102;
constexpr int kIdWarningButton = 103;
constexpr int kIdErrorButton = 104;
constexpr int kIdModalButton = 105;
constexpr int kIdModelessButton = 106;
constexpr int kIdResultLabel = 107;

constexpr int kIdInputEdit = 201;
constexpr int kIdInputOk = IDOK;      // <windows.h>で1と定義済み
constexpr int kIdInputCancel = IDCANCEL;  // <windows.h>で2と定義済み

constexpr UINT kWmModelessClosed = WM_APP + 1;  // 「モードレスダイアログが閉じた」通知

HWND g_hwndResultLabel = nullptr;
HWND g_hwndModelessDialog = nullptr;  // 開いている間だけ非nullptr
bool g_modalDialogRunning = false;    // モーダルのネストメッセージループが回っているか

void SetResultText(const wchar_t* text) {
    SetWindowTextW(g_hwndResultLabel, text);
}

void SendResultCopyData(HWND hDlg) {
    wchar_t buffer[128]{};
    GetDlgItemTextW(hDlg, kIdInputEdit, buffer, static_cast<int>(std::size(buffer)));

    COPYDATASTRUCT cds{};
    cds.dwData = IDOK;
    cds.cbData = static_cast<DWORD>((lstrlenW(buffer) + 1) * sizeof(wchar_t));
    cds.lpData = buffer;
    const HWND hOwner = GetWindow(hDlg, GW_OWNER);
    SendMessageW(hOwner, WM_COPYDATA, reinterpret_cast<WPARAM>(hDlg),
                 reinterpret_cast<LPARAM>(&cds));
}

LRESULT CALLBACK InputDialogWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            const HINSTANCE hInstance =
                reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            CreateWindowExW(0, L"STATIC", L"お名前を入力してください:", WS_CHILD | WS_VISIBLE, 12,
                             12, 200, 18, hwnd, nullptr, hInstance, nullptr);
            const HWND hwndEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                12, 34, 200, 22, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdInputEdit)),
                hInstance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"OK",
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 46, 66, 60, 24,
                             hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdInputOk)),
                             hInstance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 116, 66,
                             60, 24, hwnd,
                             reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdInputCancel)),
                             hInstance, nullptr);

            const HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            EnumChildWindows(
                hwnd,
                [](HWND child, LPARAM font) -> BOOL {
                    SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(font), TRUE);
                    return TRUE;
                },
                reinterpret_cast<LPARAM>(hFont));

            SetFocus(hwndEdit);
            return 0;
        }
        case WM_COMMAND: {
            const int id = LOWORD(wParam);
            if (id == kIdInputOk) {
                SendResultCopyData(hwnd);
                DestroyWindow(hwnd);
            } else if (id == kIdInputCancel) {
                DestroyWindow(hwnd);
            }
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY: {
            // モードレスの場合のみ、オーナーへ「閉じた」ことを通知して
            // g_hwndModelessDialogを解放させる(モーダルは呼び出し元のネストループが
            // IsWindowで自前に検知するので不要)。
            const HWND hOwner = GetWindow(hwnd, GW_OWNER);
            if (!g_modalDialogRunning) {
                PostMessageW(hOwner, kWmModelessClosed, 0, 0);
            }
            return 0;
        }
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

ATOM RegisterInputDialogClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = InputDialogWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kInputDialogClassName;
    return RegisterClassExW(&wc);
}

HWND CreateInputDialogWindow(HWND hwndOwner, HINSTANCE hInstance) {
    // WS_POPUP+WS_CAPTIONの通常ウィンドウとして作る。DS_MODALFRAME相当の見た目は
    // WS_CAPTION|WS_SYSMENUで十分再現できる。
    RECT ownerRect{};
    GetWindowRect(hwndOwner, &ownerRect);
    constexpr int kWidth = 240;
    constexpr int kHeight = 130;
    const int x = ownerRect.left + (ownerRect.right - ownerRect.left - kWidth) / 2;
    const int y = ownerRect.top + (ownerRect.bottom - ownerRect.top - kHeight) / 2;

    return CreateWindowExW(WS_EX_DLGMODALFRAME, kInputDialogClassName, L"名前を入力",
                            WS_POPUP | WS_CAPTION | WS_SYSMENU, x, y, kWidth, kHeight, hwndOwner,
                            nullptr, hInstance, nullptr);
}

void ShowModalInputDialog(HWND hwndOwner, HINSTANCE hInstance) {
    const HWND hwndDialog = CreateInputDialogWindow(hwndOwner, hInstance);
    if (hwndDialog == nullptr) {
        return;
    }

    EnableWindow(hwndOwner, FALSE);
    g_modalDialogRunning = true;
    ShowWindow(hwndDialog, SW_SHOW);

    // ネストしたメッセージループ: ここがモーダルの実体。ダイアログが閉じられて
    // ウィンドウが破棄されるまで、この関数(=呼び出し元のOnCommand)から戻らない。
    MSG msg{};
    while (IsWindow(hwndDialog) && GetMessage(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(hwndDialog, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    g_modalDialogRunning = false;
    EnableWindow(hwndOwner, TRUE);
    SetForegroundWindow(hwndOwner);
}

void ShowModelessInputDialog(HWND hwndOwner, HINSTANCE hInstance) {
    if (g_hwndModelessDialog != nullptr) {
        SetForegroundWindow(g_hwndModelessDialog);
        return;
    }
    g_hwndModelessDialog = CreateInputDialogWindow(hwndOwner, hInstance);
    ShowWindow(g_hwndModelessDialog, SW_SHOW);
    SetResultText(L"モードレスダイアログを表示中です(メインウィンドウは操作可能)");
}

void CreateChildControls(HWND hwnd, HINSTANCE hInstance) {
    struct ButtonSpec {
        int id;
        const wchar_t* text;
        int y;
    };
    const ButtonSpec buttons[] = {
        {kIdConfirmButton, L"確認ダイアログ", 16},
        {kIdInfoButton, L"情報メッセージ", 52},
        {kIdWarningButton, L"警告メッセージ", 88},
        {kIdErrorButton, L"エラーメッセージ", 124},
        {kIdModalButton, L"モーダル入力ダイアログ", 160},
        {kIdModelessButton, L"モードレス入力ダイアログ", 196},
    };
    for (const auto& spec : buttons) {
        CreateWindowExW(0, L"BUTTON", spec.text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 16, spec.y,
                         220, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(spec.id)),
                         hInstance, nullptr);
    }

    g_hwndResultLabel =
        CreateWindowExW(WS_EX_CLIENTEDGE, L"STATIC", L"(結果はここに表示されます)",
                         WS_CHILD | WS_VISIBLE | SS_LEFT, 250, 16, 220, 60, hwnd,
                         reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdResultLabel)), hInstance,
                         nullptr);

    const HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    EnumChildWindows(
        hwnd,
        [](HWND child, LPARAM font) -> BOOL {
            SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(font), TRUE);
            return TRUE;
        },
        reinterpret_cast<LPARAM>(hFont));
}

void OnCommand(HWND hwnd, HINSTANCE hInstance, WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case kIdConfirmButton: {
            const int choice = MessageBoxW(hwnd, L"本当に削除しますか?", L"確認",
                                            MB_YESNO | MB_ICONQUESTION);
            SetResultText(choice == IDYES ? L"確認: 「はい」が選択されました"
                                           : L"確認: 「いいえ」が選択されました");
            return;
        }
        case kIdInfoButton:
            MessageBoxW(hwnd, L"処理が完了しました。", L"情報", MB_OK | MB_ICONINFORMATION);
            SetResultText(L"情報メッセージを表示しました");
            return;
        case kIdWarningButton:
            MessageBoxW(hwnd, L"この操作は元に戻せません。", L"警告", MB_OK | MB_ICONWARNING);
            SetResultText(L"警告メッセージを表示しました");
            return;
        case kIdErrorButton:
            MessageBoxW(hwnd, L"処理に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
            SetResultText(L"エラーメッセージを表示しました");
            return;
        case kIdModalButton:
            ShowModalInputDialog(hwnd, hInstance);
            return;
        case kIdModelessButton:
            ShowModelessInputDialog(hwnd, hInstance);
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
            return 0;
        }
        case WM_COMMAND: {
            const HINSTANCE hInstance =
                reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            OnCommand(hwnd, hInstance, wParam);
            return 0;
        }
        case WM_COPYDATA: {
            // 入力ダイアログ(モーダル/モードレス共通)からの結果通知(OK時のみ送られてくる)。
            const auto* cds = reinterpret_cast<const COPYDATASTRUCT*>(lParam);
            const auto* text = reinterpret_cast<const wchar_t*>(cds->lpData);
            wchar_t message[220]{};
            wsprintfW(message, L"ダイアログの結果: \"%s\"", text);
            SetResultText(message);
            return TRUE;
        }
        case kWmModelessClosed:
            g_hwndModelessDialog = nullptr;
            return 0;
        case WM_DESTROY:
            if (g_hwndModelessDialog != nullptr) {
                DestroyWindow(g_hwndModelessDialog);
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
    if (RegisterMainWindowClass(hInstance) == 0 || RegisterInputDialogClass(hInstance) == 0) {
        return 0;
    }

    const HWND hwnd = CreateWindowExW(0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 520, 280, nullptr, nullptr,
                                       hInstance, nullptr);
    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        // モードレスダイアログが開いている間は、Tab移動等のダイアログ特有の
        // キー処理をIsDialogMessageに任せる(標準的なモードレスダイアログの作法)。
        if (g_hwndModelessDialog == nullptr || !IsDialogMessage(g_hwndModelessDialog, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}
