#ifndef APP_ERRORS_H
#define APP_ERRORS_H

#include <stdexcept>
#include <string>

// このアプリケーション独自の例外階層。
// すべてstd::runtime_error(ひいてはstd::exception)を継承しているため、
// 呼び出し側は必要に応じてAppError全体をまとめてcatchすることも、
// より具体的な派生型だけをcatchすることもできる。
class AppError : public std::runtime_error {
public:
    explicit AppError(const std::string& message);
};

// 設定ファイルを開けなかったときの例外。ファイルパスを保持する。
class FileOpenError : public AppError {
public:
    explicit FileOpenError(const std::string& path);

    const std::string& Path() const;

private:
    std::string path_;
};

// 設定ファイルの構文や値の変換に失敗したときの例外。行番号を保持する
// （行番号が不明な場合は-1）。
class ParseError : public AppError {
public:
    ParseError(const std::string& detail, int lineNumber);

    int LineNumber() const;

private:
    int lineNumber_;
};

// 値は変換できたが、許容範囲外だったときの例外。
class ValueOutOfRangeError : public AppError {
public:
    ValueOutOfRangeError(const std::string& key, int value, int minValue, int maxValue);

    const std::string& Key() const;
    int Value() const;

private:
    std::string key_;
    int value_;
};

#endif  // APP_ERRORS_H
