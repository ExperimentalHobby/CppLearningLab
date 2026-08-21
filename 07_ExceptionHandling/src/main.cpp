#include <iostream>
#include <stdexcept>
#include <string>

#include "app_errors.h"
#include "config_parser.h"
#include "scoped_trace.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

// try/catch/throwの基本を確認する小さなデモ。
void DemoBasicTryCatch() {
    std::cout << "=== try/catch/throwの基本 ===" << std::endl;

    auto checkAge = [](int age) {
        if (age < 0) {
            throw std::invalid_argument("age must not be negative");
        }
        return age;
    };

    try {
        std::cout << "checkAge(20) = " << checkAge(20) << std::endl;
        std::cout << "checkAge(-5) を呼び出します..." << std::endl;
        checkAge(-5);
    } catch (const std::invalid_argument& e) {
        std::cout << "catch(std::invalid_argument): " << e.what() << std::endl;
    }
}

// 設定ファイルを読み込み、port/retry_countを検証する。
// catch節はより派生した型から、より基底の型(std::exception)へ、という順序で並べるのが
// 定石（逆順だと基底のcatchが先にマッチしてしまい、派生側のcatchに到達できない）。
void ParseAndValidateConfig(const std::string& label, const std::string& path) {
    std::cout << "\n=== " << label << " ===" << std::endl;
    ScopedTrace trace("ParseAndValidateConfig(" + label + ")");
    try {
        const auto config = ParseConfigFile(path);

        for (const auto& [key, value] : config) {
            std::cout << "  " << key << " = " << value << std::endl;
        }

        if (const auto it = config.find("port"); it != config.end()) {
            const int port = ParsePositiveInt("port", it->second);
            ValidatePort(port);
            std::cout << "  -> port = " << port << " は有効です" << std::endl;
        }

        if (const auto it = config.find("retry_count"); it != config.end()) {
            const int retryCount = ParsePositiveInt("retry_count", it->second);
            std::cout << "  -> retry_count = " << retryCount << " は有効です" << std::endl;
        }
    } catch (const ValueOutOfRangeError& e) {
        // より派生した型を先にcatchする。
        std::cout << "  catch(ValueOutOfRangeError): " << e.what() << std::endl;
    } catch (const ParseError& e) {
        std::cout << "  catch(ParseError): " << e.what() << std::endl;
    } catch (const FileOpenError& e) {
        std::cout << "  catch(FileOpenError): " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // 想定していない標準例外の受け皿として、最後にstd::exceptionを置く。
        std::cout << "  catch(std::exception): " << e.what() << std::endl;
    }
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    DemoBasicTryCatch();

    const std::string dataDir = DATA_DIR;
    ParseAndValidateConfig("正常系", dataDir + "/app.config");
    ParseAndValidateConfig("存在しないファイル -> FileOpenError", dataDir + "/does_not_exist.config");
    ParseAndValidateConfig("構文エラー('='が無い行) -> ParseError",
                            dataDir + "/app_syntax_error.config");
    ParseAndValidateConfig("数値変換エラー -> ParseError", dataDir + "/app_non_numeric_port.config");
    ParseAndValidateConfig("範囲外エラー -> ValueOutOfRangeError",
                            dataDir + "/app_port_out_of_range.config");

    return 0;
}
