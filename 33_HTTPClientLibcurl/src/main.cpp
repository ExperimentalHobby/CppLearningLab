// 33. HTTPクライアント
//
// GitHub APIにGETリクエストを送り、レスポンスのJSONから必要な情報を
// 取り出して表示するCLIツール。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>

#include "http_client.h"
#include "json_value.h"

namespace {

void PrintField(const json::JsonValue& obj, const std::string& key) {
    const json::JsonValue* field = obj.Find(key);
    if (field == nullptr) {
        std::cout << "  " << key << ": (フィールドが見つかりません)\n";
        return;
    }
    if (field->type() == json::JsonType::kString) {
        std::cout << "  " << key << ": " << field->AsString() << "\n";
    } else if (field->type() == json::JsonType::kNumber) {
        std::cout << "  " << key << ": " << field->AsNumber() << "\n";
    } else if (field->type() == json::JsonType::kBool) {
        std::cout << "  " << key << ": " << (field->AsBool() ? "true" : "false") << "\n";
    } else if (field->IsNull()) {
        std::cout << "  " << key << ": null\n";
    } else {
        std::cout << "  " << key << ": (オブジェクト/配列)\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    const std::string host = "api.github.com";
    const std::string path = argc > 1 ? argv[1] : "/repos/octocat/Hello-World";
    // GitHub APIはUser-Agentヘッダーが無いリクエストを拒否するため必須。
    const std::string userAgent = "CppLearningLab-33-HTTPClient/1.0";

    std::cout << "33. HTTPクライアント\n";
    std::cout << "GET https://" << host << path << "\n\n";

    try {
        const http::HttpResponse response = http::HttpsGet(host, path, userAgent);
        std::cout << "ステータスコード: " << response.statusCode << "\n";

        if (response.statusCode != 200) {
            std::cout << "本文:\n" << response.body << "\n";
            return 1;
        }

        const json::JsonValue json = json::ParseJson(response.body);
        std::cout << "レスポンスから取り出したフィールド:\n";
        PrintField(json, "full_name");
        PrintField(json, "description");
        PrintField(json, "stargazers_count");
        PrintField(json, "language");
    } catch (const http::HttpError& e) {
        std::cerr << "HTTPエラー: " << e.what() << "\n";
        return 1;
    } catch (const json::JsonParseError& e) {
        std::cerr << "JSON解析エラー: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
