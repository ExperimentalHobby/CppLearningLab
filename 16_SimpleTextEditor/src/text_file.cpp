#include "text_file.h"

#include <windows.h>

#include <fstream>
#include <sstream>

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return L"";
    }
    const int requiredSize =
        MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(requiredSize, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), wide.data(),
                         requiredSize);
    return wide;
}

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return "";
    }
    const int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
                                                   static_cast<int>(wide.size()), nullptr, 0,
                                                   nullptr, nullptr);
    std::string utf8(requiredSize, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), utf8.data(),
                         requiredSize, nullptr, nullptr);
    return utf8;
}

bool LoadTextFile(const std::wstring& path, std::wstring* outContent) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    *outContent = Utf8ToWide(oss.str());
    return true;
}

bool SaveTextFile(const std::wstring& path, const std::wstring& content) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return false;
    }
    const std::string utf8 = WideToUtf8(content);
    file.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    return static_cast<bool>(file);
}
