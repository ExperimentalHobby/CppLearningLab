#ifndef PEN_SETTINGS_H
#define PEN_SETTINGS_H

#include <windows.h>

// 色/太さメニューのコマンドID。メニュー構築側と判定ロジック側の両方から参照するため
// ここに定義する。
constexpr int kIdColorBlack = 201;
constexpr int kIdColorRed = 202;
constexpr int kIdColorBlue = 203;
constexpr int kIdColorGreen = 204;
constexpr int kIdWidthThin = 211;
constexpr int kIdWidthMedium = 212;
constexpr int kIdWidthThick = 213;

// 現在のペン色(COLORREF)から、チェックを付けるべき色メニューのコマンドIDを判定する。
// 未知の色(メニューに無い色)が渡された場合は黒のIDを返す。GDI/HMENUに依存しない
// 純粋な判定ロジックのみを切り出しており、単体テストできる。
int ColorToMenuId(COLORREF color);

// 現在のペン太さから、チェックを付けるべき太さメニューのコマンドIDを判定する。
int WidthToMenuId(int width);

#endif  // PEN_SETTINGS_H
