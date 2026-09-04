#ifndef REFLECT_FORMAT_H
#define REFLECT_FORMAT_H

#include <string>

// 「反映」ボタン押下時に、テキストボックスの入力内容から結果ラベルへ表示する文字列を
// 組み立てる。空入力の場合は専用のメッセージにする。GetWindowTextWでコントロールから
// 読み取った後の純粋な文字列整形処理のみを切り出しており、Win32のウィンドウハンドルに
// 依存しないため単体テストできる。
std::wstring FormatReflectResult(const std::wstring& input);

// 反映回数から、カウント表示ラベルへ表示する文字列を組み立てる。
std::wstring FormatReflectCount(int count);

#endif  // REFLECT_FORMAT_H
