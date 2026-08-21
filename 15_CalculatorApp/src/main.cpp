// 15. GUI電卓
//
// ボタン入力の集約と状態管理（入力中の数値・演算子・計算結果）を持つ電卓アプリ。
// UIとロジックの分離を徹底し、電卓としての振る舞いは全て`Calculator`クラス
// (calculator.h/.cpp)に実装している。WndProcは「どのボタンが押されたか」を
// `Calculator`のメソッド呼び出しに変換し、結果を表示欄に反映するだけ。
#include <windows.h>

#include <string>

#include "calculator.h"

namespace {

constexpr wchar_t kWindowClassName[] = L"CalculatorAppClass";
constexpr wchar_t kWindowTitle[] = L"15. Calculator App - C++ Learning Lab";

constexpr int kIdDisplay = 340;
constexpr int kIdDigitBase = 300;  // 数字ボタンiのID = kIdDigitBase + i (i=0..9)
constexpr int kIdDecimalPoint = 310;
constexpr int kIdOpAdd = 320;
constexpr int kIdOpSub = 321;
constexpr int kIdOpMul = 322;
constexpr int kIdOpDiv = 323;
constexpr int kIdEquals = 330;
constexpr int kIdClear = 331;

Calculator g_calculator;
HWND g_hwndDisplay = nullptr;

void UpdateDisplay() {
    const std::string text = g_calculator.Display();
    const std::wstring wtext(text.begin(), text.end());  // 表示内容はASCIIのみなのでそのまま拡張できる
    SetWindowTextW(g_hwndDisplay, wtext.c_str());
}

void CreateChildControls(HWND hwnd, HINSTANCE hInstance) {
    g_hwndDisplay = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0",
                                     WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY, 8, 8, 232, 28,
                                     hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdDisplay)),
                                     hInstance, nullptr);

    struct ButtonSpec {
        const wchar_t* label;
        int id;
        int col;
        int row;
    };
    const ButtonSpec buttons[] = {
        {L"7", kIdDigitBase + 7, 0, 0}, {L"8", kIdDigitBase + 8, 1, 0},
        {L"9", kIdDigitBase + 9, 2, 0}, {L"+", kIdOpAdd, 3, 0},

        {L"4", kIdDigitBase + 4, 0, 1}, {L"5", kIdDigitBase + 5, 1, 1},
        {L"6", kIdDigitBase + 6, 2, 1}, {L"-", kIdOpSub, 3, 1},

        {L"1", kIdDigitBase + 1, 0, 2}, {L"2", kIdDigitBase + 2, 1, 2},
        {L"3", kIdDigitBase + 3, 2, 2}, {L"*", kIdOpMul, 3, 2},

        {L"0", kIdDigitBase + 0, 0, 3}, {L".", kIdDecimalPoint, 1, 3},
        {L"=", kIdEquals, 2, 3}, {L"/", kIdOpDiv, 3, 3},
    };

    constexpr int kButtonSize = 56;
    constexpr int kGap = 4;
    constexpr int kGridLeft = 8;
    constexpr int kGridTop = 44;

    for (const auto& spec : buttons) {
        const int x = kGridLeft + spec.col * (kButtonSize + kGap);
        const int y = kGridTop + spec.row * (kButtonSize + kGap);
        CreateWindowExW(0, L"BUTTON", spec.label, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y,
                         kButtonSize, kButtonSize, hwnd,
                         reinterpret_cast<HMENU>(static_cast<INT_PTR>(spec.id)), hInstance,
                         nullptr);
    }

    const int clearY = kGridTop + 4 * (kButtonSize + kGap);
    CreateWindowExW(0, L"BUTTON", L"C", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, kGridLeft, clearY,
                     4 * kButtonSize + 3 * kGap, 32, hwnd,
                     reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdClear)), hInstance, nullptr);

    const HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    EnumChildWindows(
        hwnd,
        [](HWND child, LPARAM font) -> BOOL {
            SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(font), TRUE);
            return TRUE;
        },
        reinterpret_cast<LPARAM>(hFont));
}

void OnCommand(WPARAM wParam) {
    const int id = LOWORD(wParam);

    if (id >= kIdDigitBase && id <= kIdDigitBase + 9) {
        g_calculator.InputDigit(static_cast<char>('0' + (id - kIdDigitBase)));
    } else {
        switch (id) {
            case kIdDecimalPoint:
                g_calculator.InputDecimalPoint();
                break;
            case kIdOpAdd:
                g_calculator.InputOperator('+');
                break;
            case kIdOpSub:
                g_calculator.InputOperator('-');
                break;
            case kIdOpMul:
                g_calculator.InputOperator('*');
                break;
            case kIdOpDiv:
                g_calculator.InputOperator('/');
                break;
            case kIdEquals:
                g_calculator.Equals();
                break;
            case kIdClear:
                g_calculator.Clear();
                break;
            default:
                return;
        }
    }
    UpdateDisplay();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            const HINSTANCE hInstance =
                reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            CreateChildControls(hwnd, hInstance);
            return 0;
        }
        case WM_COMMAND:
            OnCommand(wParam);
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
    if (RegisterMainWindowClass(hInstance) == 0) {
        return 0;
    }

    const HWND hwnd = CreateWindowExW(0, kWindowClassName, kWindowTitle,
                                       WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 270, 320, nullptr, nullptr,
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
