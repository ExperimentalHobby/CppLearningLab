// 16. 簡易テキストエディタ
//
// 複数行EDITコントロールを中心に、新規作成/開く/保存/名前を付けて保存ができる
// シンプルなメモ帳アプリ。ファイルの読み書き自体はtext_file.h/.cpp
// (Win32のウィンドウに依存しない純粋な関数)に切り出してある。
#include <windows.h>

#include <commdlg.h>

#include <iterator>
#include <string>

#include "text_file.h"

namespace {

constexpr wchar_t kWindowClassName[] = L"SimpleTextEditorClass";
constexpr wchar_t kAppTitle[] = L"16. Simple Text Editor - C++ Learning Lab";
constexpr wchar_t kUntitledName[] = L"無題";

constexpr int kIdEdit = 101;

constexpr int kIdMenuFileNew = 201;
constexpr int kIdMenuFileOpen = 202;
constexpr int kIdMenuFileSave = 203;
constexpr int kIdMenuFileSaveAs = 204;
constexpr int kIdMenuFileExit = 205;

HWND g_hwndEdit = nullptr;
std::wstring g_currentPath;  // 空文字列なら「無題」(未保存の新規ファイル)
bool g_modified = false;
bool g_suppressChangeNotification = false;  // プログラムからのSetWindowText時にEN_CHANGEを無視する

std::wstring GetEditText() {
    const int length = GetWindowTextLengthW(g_hwndEdit);
    std::wstring buffer(length, L'\0');
    if (length > 0) {
        GetWindowTextW(g_hwndEdit, buffer.data(), length + 1);
    }
    return buffer;
}

void SetEditText(const std::wstring& text) {
    g_suppressChangeNotification = true;
    SetWindowTextW(g_hwndEdit, text.c_str());
    g_suppressChangeNotification = false;
}

void UpdateTitle(HWND hwnd) {
    const std::wstring name = g_currentPath.empty() ? kUntitledName : g_currentPath;
    std::wstring title = name;
    if (g_modified) {
        title += L" *";
    }
    title += L" - ";
    title += kAppTitle;
    SetWindowTextW(hwnd, title.c_str());
}

// 未保存の変更がある場合に確認する。続行してよければtrueを返す
// (保存不要、保存済み、または「保存しない」が選ばれた場合)。
// 「キャンセル」が選ばれた場合はfalseを返し、呼び出し元は操作を中止する。
bool DoSave(HWND hwnd);  // 前方宣言

bool ConfirmDiscardChanges(HWND hwnd) {
    if (!g_modified) {
        return true;
    }
    const int choice =
        MessageBoxW(hwnd, L"変更を保存しますか?", L"確認", MB_YESNOCANCEL | MB_ICONWARNING);
    if (choice == IDCANCEL) {
        return false;
    }
    if (choice == IDYES) {
        return DoSave(hwnd);
    }
    return true;  // IDNO: 保存せず続行
}

bool ShowSaveAsDialog(HWND hwnd, std::wstring* outPath) {
    wchar_t fileBuffer[MAX_PATH] = L"";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"テキストファイル (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0";
    ofn.lpstrFile = fileBuffer;
    ofn.nMaxFile = static_cast<DWORD>(std::size(fileBuffer));
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameW(&ofn)) {
        return false;
    }
    *outPath = fileBuffer;
    return true;
}

bool ShowOpenDialog(HWND hwnd, std::wstring* outPath) {
    wchar_t fileBuffer[MAX_PATH] = L"";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"テキストファイル (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0";
    ofn.lpstrFile = fileBuffer;
    ofn.nMaxFile = static_cast<DWORD>(std::size(fileBuffer));
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) {
        return false;
    }
    *outPath = fileBuffer;
    return true;
}

bool DoSaveAs(HWND hwnd) {
    std::wstring path;
    if (!ShowSaveAsDialog(hwnd, &path)) {
        return false;
    }
    if (!SaveTextFile(path, GetEditText())) {
        MessageBoxW(hwnd, L"ファイルを保存できませんでした。", L"エラー", MB_OK | MB_ICONERROR);
        return false;
    }
    g_currentPath = path;
    g_modified = false;
    UpdateTitle(hwnd);
    return true;
}

bool DoSave(HWND hwnd) {
    if (g_currentPath.empty()) {
        return DoSaveAs(hwnd);
    }
    if (!SaveTextFile(g_currentPath, GetEditText())) {
        MessageBoxW(hwnd, L"ファイルを保存できませんでした。", L"エラー", MB_OK | MB_ICONERROR);
        return false;
    }
    g_modified = false;
    UpdateTitle(hwnd);
    return true;
}

void DoNew(HWND hwnd) {
    if (!ConfirmDiscardChanges(hwnd)) {
        return;
    }
    SetEditText(L"");
    g_currentPath.clear();
    g_modified = false;
    UpdateTitle(hwnd);
}

void DoOpen(HWND hwnd) {
    if (!ConfirmDiscardChanges(hwnd)) {
        return;
    }
    std::wstring path;
    if (!ShowOpenDialog(hwnd, &path)) {
        return;
    }
    std::wstring content;
    if (!LoadTextFile(path, &content)) {
        MessageBoxW(hwnd, L"ファイルを開けませんでした。", L"エラー", MB_OK | MB_ICONERROR);
        return;
    }
    SetEditText(content);
    g_currentPath = path;
    g_modified = false;
    UpdateTitle(hwnd);
}

HMENU BuildMenuBar() {
    HMENU hMenuBar = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, kIdMenuFileNew, L"新規作成(&N)\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, kIdMenuFileOpen, L"開く(&O)...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_STRING, kIdMenuFileSave, L"上書き保存(&S)\tCtrl+S");
    AppendMenuW(hFileMenu, MF_STRING, kIdMenuFileSaveAs, L"名前を付けて保存(&A)...");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFileMenu, MF_STRING, kIdMenuFileExit, L"終了(&X)");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), L"ファイル(&F)");
    return hMenuBar;
}

void OnCommand(HWND hwnd, WPARAM wParam) {
    const int id = LOWORD(wParam);
    const int notificationCode = HIWORD(wParam);

    if (id == kIdEdit && notificationCode == EN_CHANGE) {
        if (!g_suppressChangeNotification && !g_modified) {
            g_modified = true;
            UpdateTitle(hwnd);
        }
        return;
    }

    switch (id) {
        case kIdMenuFileNew:
            DoNew(hwnd);
            return;
        case kIdMenuFileOpen:
            DoOpen(hwnd);
            return;
        case kIdMenuFileSave:
            DoSave(hwnd);
            return;
        case kIdMenuFileSaveAs:
            DoSaveAs(hwnd);
            return;
        case kIdMenuFileExit:
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
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
            g_hwndEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL |
                    ES_WANTRETURN,
                0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdEdit)),
                hInstance, nullptr);
            SendMessageW(g_hwndEdit, WM_SETFONT,
                         reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
            SetMenu(hwnd, BuildMenuBar());
            UpdateTitle(hwnd);
            return 0;
        }
        case WM_SIZE: {
            RECT clientRect{};
            GetClientRect(hwnd, &clientRect);
            MoveWindow(g_hwndEdit, 0, 0, clientRect.right, clientRect.bottom, TRUE);
            return 0;
        }
        case WM_COMMAND:
            OnCommand(hwnd, wParam);
            return 0;
        case WM_CLOSE:
            if (ConfirmDiscardChanges(hwnd)) {
                DestroyWindow(hwnd);
            }
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
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kWindowClassName;
    return RegisterClassExW(&wc);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, PWSTR /*pCmdLine*/,
                     int nCmdShow) {
    if (RegisterMainWindowClass(hInstance) == 0) {
        return 0;
    }

    const HWND hwnd = CreateWindowExW(0, kWindowClassName, kAppTitle, WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr,
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
