// 17. 簡易お絵かきアプリ
//
// マウスの左ボタンドラッグで自由線を描ける簡易ペイントアプリ。
// 実際の描画内容は「メモリDC上のビットマップ」に保持し、WM_PAINTでは
// そのビットマップを画面へBitBltでコピーするだけにする(ダブルバッファリング)。
// こうすることで、ウィンドウを他のウィンドウで隠して戻したときなど、
// 再描画が必要になっても描いた内容が消えない。
#include <windows.h>

#include <windowsx.h>  // GET_X_LPARAM/GET_Y_LPARAM

#include <initializer_list>

#include "pen_settings.h"

namespace {

constexpr wchar_t kWindowClassName[] = L"PaintAppClass";
constexpr wchar_t kWindowTitle[] = L"17. Paint App - C++ Learning Lab";

constexpr int kIdClear = 220;

HDC g_hdcMem = nullptr;      // 描画内容を保持するメモリDC
HBITMAP g_hbmMem = nullptr;  // メモリDCに選択中のビットマップ
HBITMAP g_hbmOld = nullptr;  // 元々メモリDCに入っていた(1x1の)既定ビットマップ

COLORREF g_penColor = RGB(0, 0, 0);
int g_penWidth = 2;
bool g_isDrawing = false;
POINT g_lastPoint{};

// クライアント領域のサイズに合わせてメモリDC上のビットマップを(再)作成し、白で塗る。
// 簡易実装のため、リサイズ時は既存の描画内容を引き継がず白紙に戻る
// (実用アプリではリサイズ前のビットマップからBitBltでコピーする必要がある)。
void RecreateCanvas(HWND hwnd) {
    RECT clientRect{};
    GetClientRect(hwnd, &clientRect);
    const int width = (clientRect.right > 0) ? clientRect.right : 1;
    const int height = (clientRect.bottom > 0) ? clientRect.bottom : 1;

    const HDC hdcWindow = GetDC(hwnd);

    if (g_hdcMem == nullptr) {
        g_hdcMem = CreateCompatibleDC(hdcWindow);
    }
    if (g_hbmMem != nullptr) {
        SelectObject(g_hdcMem, g_hbmOld);
        DeleteObject(g_hbmMem);
    }
    g_hbmMem = CreateCompatibleBitmap(hdcWindow, width, height);
    g_hbmOld = static_cast<HBITMAP>(SelectObject(g_hdcMem, g_hbmMem));

    RECT fillRect{0, 0, width, height};
    FillRect(g_hdcMem, &fillRect, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

    ReleaseDC(hwnd, hdcWindow);
}

void DrawSegment(HWND hwnd, POINT from, POINT to) {
    const HPEN hPen = CreatePen(PS_SOLID, g_penWidth, g_penColor);
    const HPEN hOldPen = static_cast<HPEN>(SelectObject(g_hdcMem, hPen));

    MoveToEx(g_hdcMem, from.x, from.y, nullptr);
    LineTo(g_hdcMem, to.x, to.y);

    SelectObject(g_hdcMem, hOldPen);
    DeleteObject(hPen);

    // 変化した範囲だけ再描画すれば十分だが、単純さのためウィンドウ全体を無効化する。
    InvalidateRect(hwnd, nullptr, FALSE);
}

void ClearCanvas(HWND hwnd) {
    RECT clientRect{};
    GetClientRect(hwnd, &clientRect);
    FillRect(g_hdcMem, &clientRect, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    InvalidateRect(hwnd, nullptr, FALSE);
}

HMENU BuildMenuBar() {
    HMENU hMenuBar = CreateMenu();

    HMENU hColorMenu = CreatePopupMenu();
    AppendMenuW(hColorMenu, MF_STRING, kIdColorBlack, L"黒(&B)");
    AppendMenuW(hColorMenu, MF_STRING, kIdColorRed, L"赤(&R)");
    AppendMenuW(hColorMenu, MF_STRING, kIdColorBlue, L"青(&U)");
    AppendMenuW(hColorMenu, MF_STRING, kIdColorGreen, L"緑(&G)");
    CheckMenuItem(hColorMenu, kIdColorBlack, MF_BYCOMMAND | MF_CHECKED);
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hColorMenu), L"色(&C)");

    HMENU hWidthMenu = CreatePopupMenu();
    AppendMenuW(hWidthMenu, MF_STRING, kIdWidthThin, L"細い(&1)");
    AppendMenuW(hWidthMenu, MF_STRING, kIdWidthMedium, L"普通(&2)");
    AppendMenuW(hWidthMenu, MF_STRING, kIdWidthThick, L"太い(&3)");
    CheckMenuItem(hWidthMenu, kIdWidthMedium, MF_BYCOMMAND | MF_CHECKED);
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hWidthMenu), L"太さ(&W)");

    HMENU hEditMenu = CreatePopupMenu();
    AppendMenuW(hEditMenu, MF_STRING, kIdClear, L"クリア(&L)");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hEditMenu), L"編集(&E)");

    return hMenuBar;
}

// 選択中の色/太さのメニューにチェックを付け直す。
void UpdateMenuChecks(HWND hwnd) {
    const HMENU hMenuBar = GetMenu(hwnd);
    const HMENU hColorMenu = GetSubMenu(hMenuBar, 0);
    const HMENU hWidthMenu = GetSubMenu(hMenuBar, 1);

    for (const int id : {kIdColorBlack, kIdColorRed, kIdColorBlue, kIdColorGreen}) {
        CheckMenuItem(hColorMenu, id, MF_BYCOMMAND | MF_UNCHECKED);
    }
    for (const int id : {kIdWidthThin, kIdWidthMedium, kIdWidthThick}) {
        CheckMenuItem(hWidthMenu, id, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // 色/太さからチェック対象メニューIDを判定するロジックはpen_settings.cppに
    // 切り出してあり、単体テストで検証済み。
    CheckMenuItem(hColorMenu, ColorToMenuId(g_penColor), MF_BYCOMMAND | MF_CHECKED);
    CheckMenuItem(hWidthMenu, WidthToMenuId(g_penWidth), MF_BYCOMMAND | MF_CHECKED);
}

void OnCommand(HWND hwnd, WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case kIdColorBlack:
            g_penColor = RGB(0, 0, 0);
            break;
        case kIdColorRed:
            g_penColor = RGB(255, 0, 0);
            break;
        case kIdColorBlue:
            g_penColor = RGB(0, 0, 255);
            break;
        case kIdColorGreen:
            g_penColor = RGB(0, 160, 0);
            break;
        case kIdWidthThin:
            g_penWidth = 1;
            break;
        case kIdWidthMedium:
            g_penWidth = 3;
            break;
        case kIdWidthThick:
            g_penWidth = 6;
            break;
        case kIdClear:
            ClearCanvas(hwnd);
            return;
        default:
            return;
    }
    UpdateMenuChecks(hwnd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            SetMenu(hwnd, BuildMenuBar());
            RecreateCanvas(hwnd);
            return 0;
        case WM_SIZE:
            RecreateCanvas(hwnd);
            return 0;
        case WM_COMMAND:
            OnCommand(hwnd, wParam);
            return 0;
        case WM_LBUTTONDOWN:
            g_isDrawing = true;
            g_lastPoint.x = GET_X_LPARAM(lParam);
            g_lastPoint.y = GET_Y_LPARAM(lParam);
            SetCapture(hwnd);  // ウィンドウ外にマウスが出てもドラッグを継続できるようにする
            return 0;
        case WM_MOUSEMOVE:
            if (g_isDrawing && (wParam & MK_LBUTTON)) {
                const POINT current{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                DrawSegment(hwnd, g_lastPoint, current);
                g_lastPoint = current;
            }
            return 0;
        case WM_LBUTTONUP:
            g_isDrawing = false;
            ReleaseCapture();
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps{};
            const HDC hdc = BeginPaint(hwnd, &ps);
            RECT clientRect{};
            GetClientRect(hwnd, &clientRect);
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, g_hdcMem, 0, 0, SRCCOPY);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            if (g_hbmMem != nullptr) {
                SelectObject(g_hdcMem, g_hbmOld);
                DeleteObject(g_hbmMem);
            }
            if (g_hdcMem != nullptr) {
                DeleteDC(g_hdcMem);
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
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
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
