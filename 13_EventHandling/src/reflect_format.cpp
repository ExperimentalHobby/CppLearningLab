#include "reflect_format.h"

std::wstring FormatReflectResult(const std::wstring& input) {
    if (input.empty()) {
        return L"(空の入力が反映されました)";
    }
    return input;
}

std::wstring FormatReflectCount(int count) {
    return L"反映回数: " + std::to_wstring(count);
}
