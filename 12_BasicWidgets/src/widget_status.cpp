#include "widget_status.h"

std::wstring FormatApplyStatus(const std::wstring& name, bool notifyEnabled) {
    return L"適用: 名前=\"" + name + L"\", 通知=" + (notifyEnabled ? L"ON" : L"OFF");
}
