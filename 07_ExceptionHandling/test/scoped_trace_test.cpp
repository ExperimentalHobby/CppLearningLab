#include "scoped_trace.h"

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
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

TEST(ScopedTraceTest, LogsStartThenEndOnNormalScopeExit) {
    CoutRedirect redirect;
    {
        ScopedTrace trace("normal");
    }

    const std::string output = redirect.str();
    const auto start_pos = output.find("開始: normal");
    const auto end_pos = output.find("終了: normal");

    ASSERT_NE(start_pos, std::string::npos);
    ASSERT_NE(end_pos, std::string::npos);
    EXPECT_LT(start_pos, end_pos);
}

// 例外がスコープを通り抜けて巻き戻る(スタックアンワインド)ときも、デストラクタは
// 必ず呼ばれるため「終了」ログが出力される。
TEST(ScopedTraceTest, LogsEndEvenWhenExceptionUnwindsScope) {
    CoutRedirect redirect;
    try {
        ScopedTrace trace("unwind");
        throw std::runtime_error("boom");
    } catch (const std::runtime_error&) {
        // 例外はここで飲み込む。ScopedTraceのデストラクタは既に呼ばれているはず。
    }

    const std::string output = redirect.str();
    EXPECT_NE(output.find("終了: unwind"), std::string::npos);
}
