#ifndef TEXT_FILE_H
#define TEXT_FILE_H

#include <string>

// UTF-16(std::wstring、Win32のEDITコントロールが扱う形式)と
// UTF-8(std::string、ファイルに保存する形式)の相互変換。
std::wstring Utf8ToWide(const std::string& utf8);
std::string WideToUtf8(const std::wstring& wide);

// UTF-8テキストファイルの読み書き。ウィンドウ/コントロールを一切使わないので、
// GUIを起動せずに単体でテストできる。失敗した場合はfalseを返す。
bool LoadTextFile(const std::wstring& path, std::wstring* outContent);
bool SaveTextFile(const std::wstring& path, const std::wstring& content);

#endif  // TEXT_FILE_H
