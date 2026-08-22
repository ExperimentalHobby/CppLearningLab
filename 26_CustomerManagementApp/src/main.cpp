// 26. 顧客管理アプリ（GUI+DB統合）
//
// 11-18番のGUI(Win32)と21番のDB連携(SQLite)を統合した集大成的な課題。
// UI層(このファイル)はCustomerRepository(customer_repository.h/.cpp、
// Win32非依存)を通じてのみDBを操作し、UI層とデータ層を分離している。
#include <windows.h>

#include <commctrl.h>

#include <iterator>
#include <string>
#include <vector>

#include "customer_repository.h"

#pragma comment(lib, "comctl32.lib")

namespace {

constexpr wchar_t kWindowClassName[] = L"CustomerManagementAppClass";
constexpr wchar_t kWindowTitle[] = L"26. Customer Management App - C++ Learning Lab";

constexpr int kIdListView = 101;
constexpr int kIdEditName = 102;
constexpr int kIdEditPhone = 103;
constexpr int kIdEditEmail = 104;
constexpr int kIdEditSearch = 105;
constexpr int kIdButtonAdd = 110;
constexpr int kIdButtonUpdate = 111;
constexpr int kIdButtonDelete = 112;
constexpr int kIdButtonClear = 113;
constexpr int kIdButtonSearch = 114;
constexpr int kIdButtonShowAll = 115;
constexpr int kIdStatusLabel = 120;

HWND g_hwndList = nullptr;
HWND g_hwndName = nullptr;
HWND g_hwndPhone = nullptr;
HWND g_hwndEmail = nullptr;
HWND g_hwndSearch = nullptr;
HWND g_hwndStatus = nullptr;

customer::CustomerRepository g_repository;
std::vector<customer::Customer> g_lastResults;
long long g_selectedId = 0;  // 0は「未選択」を表す

// --- UTF-8 <-> UTF-16 変換 ---
// CustomerRepositoryはUTF-8のstd::stringでやり取りするため、Win32のW系API
// (wchar_t/UTF-16)との境界でここだけ変換する(16_SimpleTextEditorと同じ考え方)。
std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return {};
    }
    const int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), wide.data(), len);
    return wide;
}

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return {};
    }
    const int len =
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    std::string utf8(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), utf8.data(), len, nullptr, nullptr);
    return utf8;
}

std::wstring GetEditText(HWND hEdit) {
    const int len = GetWindowTextLengthW(hEdit);
    std::wstring buffer(static_cast<size_t>(len), L'\0');
    if (len > 0) {
        GetWindowTextW(hEdit, buffer.data(), len + 1);
    }
    return buffer;
}

void SetStatus(const wchar_t* text) { SetWindowTextW(g_hwndStatus, text); }

void SetupListViewColumns() {
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    struct ColumnDef {
        const wchar_t* title;
        int width;
    };
    const ColumnDef columns[] = {
        {L"ID", 40},
        {L"氏名", 140},
        {L"電話番号", 140},
        {L"メール", 200},
    };
    for (int i = 0; i < static_cast<int>(std::size(columns)); ++i) {
        col.pszText = const_cast<wchar_t*>(columns[i].title);
        col.cx = columns[i].width;
        col.iSubItem = i;
        ListView_InsertColumn(g_hwndList, i, &col);
    }
}

// リストを再描画し、g_lastResultsを最新の検索結果で更新する。
// 各行のlParamに顧客idを持たせることで、選択された行から元のCustomerを
// 逆引きできるようにしている(表示用の文字列から再パースする必要がない)。
void RefreshList(const std::wstring& nameFilter) {
    try {
        g_lastResults = g_repository.Search(WideToUtf8(nameFilter));
    } catch (const customer::CustomerRepositoryError& e) {
        SetStatus(Utf8ToWide(std::string("検索エラー: ") + e.what()).c_str());
        return;
    }

    ListView_DeleteAllItems(g_hwndList);
    for (size_t i = 0; i < g_lastResults.size(); ++i) {
        const customer::Customer& c = g_lastResults[i];
        LVITEMW item{};
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = static_cast<int>(i);
        item.iSubItem = 0;
        const std::wstring idText = std::to_wstring(c.id);
        item.pszText = const_cast<wchar_t*>(idText.c_str());
        item.lParam = static_cast<LPARAM>(c.id);
        const int row = ListView_InsertItem(g_hwndList, &item);

        const std::wstring name = Utf8ToWide(c.name);
        const std::wstring phone = Utf8ToWide(c.phone);
        const std::wstring email = Utf8ToWide(c.email);
        ListView_SetItemText(g_hwndList, row, 1, const_cast<wchar_t*>(name.c_str()));
        ListView_SetItemText(g_hwndList, row, 2, const_cast<wchar_t*>(phone.c_str()));
        ListView_SetItemText(g_hwndList, row, 3, const_cast<wchar_t*>(email.c_str()));
    }

    const std::wstring statusText = L"表示件数: " + std::to_wstring(g_lastResults.size()) + L"件";
    SetStatus(statusText.c_str());
}

void ClearForm() {
    SetWindowTextW(g_hwndName, L"");
    SetWindowTextW(g_hwndPhone, L"");
    SetWindowTextW(g_hwndEmail, L"");
    g_selectedId = 0;
    ListView_SetItemState(g_hwndList, -1, 0, LVIS_SELECTED);
}

// 選択された行のlParam(顧客id)から、g_lastResults内の該当Customerを探して
// 入力欄に反映する。
void OnSelectionChanged(int itemIndex) {
    if (itemIndex < 0) {
        return;
    }
    LVITEMW item{};
    item.mask = LVIF_PARAM;
    item.iItem = itemIndex;
    if (!ListView_GetItem(g_hwndList, &item)) {
        return;
    }
    const long long id = static_cast<long long>(item.lParam);
    for (const customer::Customer& c : g_lastResults) {
        if (c.id == id) {
            g_selectedId = id;
            SetWindowTextW(g_hwndName, Utf8ToWide(c.name).c_str());
            SetWindowTextW(g_hwndPhone, Utf8ToWide(c.phone).c_str());
            SetWindowTextW(g_hwndEmail, Utf8ToWide(c.email).c_str());
            return;
        }
    }
}

void OnAdd() {
    customer::Customer c;
    c.name = WideToUtf8(GetEditText(g_hwndName));
    c.phone = WideToUtf8(GetEditText(g_hwndPhone));
    c.email = WideToUtf8(GetEditText(g_hwndEmail));
    if (c.name.empty()) {
        SetStatus(L"氏名を入力してください。");
        return;
    }
    try {
        const long long id = g_repository.Add(c);
        SetStatus((L"追加しました(#" + std::to_wstring(id) + L")").c_str());
        ClearForm();
        RefreshList(GetEditText(g_hwndSearch));
    } catch (const customer::CustomerRepositoryError& e) {
        SetStatus(Utf8ToWide(std::string("追加エラー: ") + e.what()).c_str());
    }
}

void OnUpdate() {
    if (g_selectedId == 0) {
        SetStatus(L"更新する行をリストから選択してください。");
        return;
    }
    customer::Customer c;
    c.id = g_selectedId;
    c.name = WideToUtf8(GetEditText(g_hwndName));
    c.phone = WideToUtf8(GetEditText(g_hwndPhone));
    c.email = WideToUtf8(GetEditText(g_hwndEmail));
    if (c.name.empty()) {
        SetStatus(L"氏名を入力してください。");
        return;
    }
    try {
        const bool ok = g_repository.Update(c);
        SetStatus(ok ? L"更新しました。" : L"該当する顧客が見つかりません。");
        RefreshList(GetEditText(g_hwndSearch));
    } catch (const customer::CustomerRepositoryError& e) {
        SetStatus(Utf8ToWide(std::string("更新エラー: ") + e.what()).c_str());
    }
}

void OnDelete() {
    if (g_selectedId == 0) {
        SetStatus(L"削除する行をリストから選択してください。");
        return;
    }
    try {
        const bool ok = g_repository.Remove(g_selectedId);
        SetStatus(ok ? L"削除しました。" : L"該当する顧客が見つかりません。");
        ClearForm();
        RefreshList(GetEditText(g_hwndSearch));
    } catch (const customer::CustomerRepositoryError& e) {
        SetStatus(Utf8ToWide(std::string("削除エラー: ") + e.what()).c_str());
    }
}

void CreateControls(HWND hwnd, HINSTANCE hInstance) {
    const HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    auto createLabel = [&](const wchar_t* text, int x, int y, int w) {
        HWND h = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, 20, hwnd, nullptr,
                                  hInstance, nullptr);
        SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    };
    auto createEdit = [&](int id, int x, int y, int w) {
        HWND h = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, x, y,
                                  w, 22, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInstance,
                                  nullptr);
        SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
        return h;
    };
    auto createButton = [&](const wchar_t* text, int id, int x, int y, int w) {
        HWND h = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, w, 26, hwnd,
                                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInstance, nullptr);
        SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    };

    createLabel(L"氏名:", 16, 16, 60);
    g_hwndName = createEdit(kIdEditName, 80, 14, 180);
    createLabel(L"電話番号:", 16, 46, 60);
    g_hwndPhone = createEdit(kIdEditPhone, 80, 44, 180);
    createLabel(L"メール:", 16, 76, 60);
    g_hwndEmail = createEdit(kIdEditEmail, 80, 74, 180);

    createButton(L"追加", kIdButtonAdd, 280, 14, 80);
    createButton(L"更新", kIdButtonUpdate, 280, 44, 80);
    createButton(L"削除", kIdButtonDelete, 280, 74, 80);
    createButton(L"クリア", kIdButtonClear, 370, 74, 80);

    createLabel(L"検索(氏名):", 16, 112, 70);
    g_hwndSearch = createEdit(kIdEditSearch, 90, 110, 170);
    createButton(L"検索", kIdButtonSearch, 270, 108, 70);
    createButton(L"全件表示", kIdButtonShowAll, 350, 108, 90);

    g_hwndList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                  WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER, 16, 146, 500,
                                  200, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdListView)),
                                  hInstance, nullptr);
    ListView_SetExtendedListViewStyle(g_hwndList, LVS_EX_FULLROWSELECT);
    SetupListViewColumns();

    g_hwndStatus = CreateWindowExW(0, L"STATIC", L"準備完了", WS_CHILD | WS_VISIBLE, 16, 356, 500, 20, hwnd,
                                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusLabel)), hInstance,
                                    nullptr);
    SendMessageW(g_hwndStatus, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            const HINSTANCE hInstance =
                reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            CreateControls(hwnd, hInstance);
            try {
                g_repository.Open("customers.db");
                RefreshList(L"");
            } catch (const customer::CustomerRepositoryError& e) {
                SetStatus(Utf8ToWide(std::string("初期化エラー: ") + e.what()).c_str());
            }
            return 0;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case kIdButtonAdd:
                    OnAdd();
                    break;
                case kIdButtonUpdate:
                    OnUpdate();
                    break;
                case kIdButtonDelete:
                    OnDelete();
                    break;
                case kIdButtonClear:
                    ClearForm();
                    break;
                case kIdButtonSearch:
                    RefreshList(GetEditText(g_hwndSearch));
                    break;
                case kIdButtonShowAll:
                    SetWindowTextW(g_hwndSearch, L"");
                    RefreshList(L"");
                    break;
                default:
                    break;
            }
            return 0;
        }
        case WM_NOTIFY: {
            const NMHDR* header = reinterpret_cast<NMHDR*>(lParam);
            if (header->idFrom == static_cast<UINT_PTR>(kIdListView) && header->code == LVN_ITEMCHANGED) {
                const NMLISTVIEW* nmlv = reinterpret_cast<const NMLISTVIEW*>(lParam);
                if ((nmlv->uNewState & LVIS_SELECTED) != 0) {
                    OnSelectionChanged(nmlv->iItem);
                }
            }
            return 0;
        }
        case WM_DESTROY:
            g_repository.Close();
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, PWSTR /*pCmdLine*/, int nCmdShow) {
    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    if (RegisterMainWindowClass(hInstance) == 0) {
        return 0;
    }

    const HWND hwnd = CreateWindowExW(0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                       CW_USEDEFAULT, 560, 430, nullptr, nullptr, hInstance, nullptr);
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
