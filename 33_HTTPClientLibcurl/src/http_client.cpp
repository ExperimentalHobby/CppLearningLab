#include "http_client.h"

#include <windows.h>

#include <winhttp.h>

#include <vector>

namespace http {

namespace {

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return {};
    }
    const int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(),
                                        static_cast<int>(utf8.size()), nullptr, 0);
    if (len <= 0) {
        throw HttpError("MultiByteToWideCharに失敗しました: エラーコード=" + std::to_string(GetLastError()));
    }
    std::wstring wide(static_cast<size_t>(len), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), static_cast<int>(utf8.size()), wide.data(),
                            len) <= 0) {
        throw HttpError("MultiByteToWideCharに失敗しました: エラーコード=" + std::to_string(GetLastError()));
    }
    return wide;
}

// HINTERNETハンドルをRAIIで後始末する。
class WinHttpHandle {
   public:
    explicit WinHttpHandle(HINTERNET handle) : handle_(handle) {}
    ~WinHttpHandle() {
        if (handle_ != nullptr) {
            WinHttpCloseHandle(handle_);
        }
    }
    WinHttpHandle(const WinHttpHandle&) = delete;
    WinHttpHandle& operator=(const WinHttpHandle&) = delete;

    HINTERNET get() const { return handle_; }
    explicit operator bool() const { return handle_ != nullptr; }

   private:
    HINTERNET handle_;
};

}  // namespace

HttpResponse HttpsGet(const std::string& host, const std::string& path, const std::string& userAgent) {
    const std::wstring wideHost = Utf8ToWide(host);
    const std::wstring widePath = Utf8ToWide(path);
    const std::wstring wideUserAgent = Utf8ToWide(userAgent);

    WinHttpHandle session(WinHttpOpen(wideUserAgent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                       WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session) {
        throw HttpError("WinHttpOpenに失敗しました: エラーコード=" + std::to_string(GetLastError()));
    }

    WinHttpHandle connection(
        WinHttpConnect(session.get(), wideHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0));
    if (!connection) {
        throw HttpError("WinHttpConnectに失敗しました: エラーコード=" + std::to_string(GetLastError()));
    }

    WinHttpHandle request(WinHttpOpenRequest(connection.get(), L"GET", widePath.c_str(), nullptr,
                                              WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                              WINHTTP_FLAG_SECURE));
    if (!request) {
        throw HttpError("WinHttpOpenRequestに失敗しました: エラーコード=" + std::to_string(GetLastError()));
    }

    const BOOL sendOk = WinHttpSendRequest(request.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!sendOk) {
        throw HttpError("WinHttpSendRequestに失敗しました: エラーコード=" + std::to_string(GetLastError()));
    }

    if (!WinHttpReceiveResponse(request.get(), nullptr)) {
        throw HttpError("WinHttpReceiveResponseに失敗しました: エラーコード=" + std::to_string(GetLastError()));
    }

    HttpResponse response;

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(request.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
    response.statusCode = static_cast<int>(statusCode);

    for (;;) {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request.get(), &available)) {
            throw HttpError("WinHttpQueryDataAvailableに失敗しました: エラーコード=" +
                             std::to_string(GetLastError()));
        }
        if (available == 0) {
            break;
        }
        std::vector<char> buffer(available);
        DWORD bytesRead = 0;
        if (!WinHttpReadData(request.get(), buffer.data(), available, &bytesRead)) {
            throw HttpError("WinHttpReadDataに失敗しました: エラーコード=" + std::to_string(GetLastError()));
        }
        response.body.append(buffer.data(), bytesRead);
    }

    return response;
}

}  // namespace http
