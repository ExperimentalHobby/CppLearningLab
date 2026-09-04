// 11. ウィンドウ表示の基本
//
// GUIアプリケーションの最小構成: ウィンドウクラスを登録し、ウィンドウを1枚作成して
// メッセージループを回すだけのプログラム。以降のGUI課題(12〜18)も基本的に
// この骨格(WinMain -> RegisterClassEx -> CreateWindowEx -> メッセージループ)の上に
// コントロールや処理を追加していく。
#include <windows.h>

namespace {

constexpr wchar_t kWindowClassName[] = L"HelloWindowClass";
constexpr wchar_t kWindowTitle[] = L"11. Hello Window - C++ Learning Lab";

// ウィンドウプロシージャ: このウィンドウ宛てのメッセージ(イベント)を処理する。
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            // ウィンドウが破棄された(閉じるボタンが押された等)ら、メッセージループを
            // 終了させるためにWM_QUITを送る。
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

// ウィンドウクラス(このアプリのウィンドウの「種類」)をOSに登録する。
// CreateWindowExWで実際にウィンドウを作る前に、必ずこの登録が必要。
ATOM RegisterMainWindowClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    // CS_HREDRAW/CS_VREDRAW: 幅または高さが変わったらクライアント領域全体を
    // 再描画する(このアプリでは表示内容がないため実質影響しないが、GUI課題共通の慣習)。
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    // COLOR_WINDOW+1: GDIのブラシ判定用に1を足す(0はNULL_BRUSHと区別が付かないため)。
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kWindowClassName;
    return RegisterClassExW(&wc);
}

}  // namespace

// GUIアプリのエントリポイント。コンソールアプリのmain()に相当するが、
// 引数でインスタンスハンドルや表示状態(最小化/最大化等)を受け取る点が異なる。
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, PWSTR /*pCmdLine*/,
                     int nCmdShow) {
    if (RegisterMainWindowClass(hInstance) == 0) {
        return 0;
    }

    // 実際のウィンドウを1枚作成する。WS_OVERLAPPEDWINDOWは、タイトルバー・最小化/
    // 最大化ボタン・サイズ変更枠を備えた通常のトップレベルウィンドウの標準スタイル。
    const HWND hwnd = CreateWindowExW(0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 480, 320, nullptr, nullptr,
                                       hInstance, nullptr);
    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // メッセージループ: OSからのメッセージ(イベント)を取り出し、WndProcへ振り分け続ける。
    // WM_QUIT(PostQuitMessageで送られる)を受け取るとGetMessageが0を返しループを抜ける。
    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
