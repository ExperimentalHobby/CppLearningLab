// 意図的に排他制御をしていないカウンタ。データ競合(race condition)を
// 実演するための比較対象であり、複数スレッドから同時にIncrement()すると
// 「読み取り→加算→書き戻し」の間に他スレッドが割り込んで更新が消える
// (lost update)ことがあり、最終値が期待より小さくなる。
//
// この不正確さ自体がこの課題の学習ポイントであり、非決定的で実行のたびに
// 結果が変わりうるため単体テストの対象にはせず、main.cppのデモでのみ使う。
#pragma once

#include <cstdint>

namespace concurrency {

class UnsafeCounter {
   public:
    void Increment() { ++value_; }
    int64_t Value() const { return value_; }

   private:
    int64_t value_ = 0;
};

}  // namespace concurrency
