#include "scoped_resource.h"

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

namespace {

// std::coutのstreambufを一時的にostringstreamへ差し替え、出力内容を文字列として
// 検証できるようにするヘルパー。デストラクタで必ず元のstreambufに戻す(RAII)。
class CoutRedirect {
public:
    CoutRedirect() : old_buf_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~CoutRedirect() { std::cout.rdbuf(old_buf_); }

    std::string str() const { return buffer_.str(); }

private:
    std::ostringstream buffer_;
    std::streambuf* old_buf_;
};

}  // namespace

// コンストラクタで「取得」、デストラクタで「解放」のログが、スコープを抜けたタイミングで
// この順番に出力されることを確認する。
TEST(ScopedResourceTest, LogsAcquireThenReleaseOnScopeExit) {
    CoutRedirect redirect;
    {
        ScopedResource resource("file.txt");
    }  // ここでデストラクタが呼ばれ、解放ログが出力されるはず。

    const std::string output = redirect.str();
    const auto acquire_pos = output.find("取得: file.txt");
    const auto release_pos = output.find("解放: file.txt");

    ASSERT_NE(acquire_pos, std::string::npos);
    ASSERT_NE(release_pos, std::string::npos);
    EXPECT_LT(acquire_pos, release_pos);
}
