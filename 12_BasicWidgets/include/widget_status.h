#ifndef WIDGET_STATUS_H
#define WIDGET_STATUS_H

#include <string>

// 「適用」ボタン押下時に、名前欄の入力内容とチェックボックスの状態から
// ステータスバーへ表示する文字列を組み立てる。GetWindowTextW/SendMessageWで
// コントロールから値を読み取った後の純粋な文字列整形処理のみを切り出しており、
// Win32のウィンドウハンドルに依存しないため単体テストできる。
std::wstring FormatApplyStatus(const std::wstring& name, bool notifyEnabled);

#endif  // WIDGET_STATUS_H
