// 意図的に排他制御をしていないカウンタ。データ競合(race condition)を
// 実演するための比較対象であり、複数スレッドから同時にIncrement()すると
// 「読み取り→加算→書き戻し」の間に他スレッドが割り込んで更新が消える
// (lost update)ことがあり、最終値が期待より小さくなる。
//
// 厳密には、C++のメモリモデル上はこのようなデータ競合が発生した時点で
// プログラム全体の動作が未定義動作(undefined behavior)になる。実際には
// 「値が期待より小さくなる」程度で済むことが多いが、規格上はそれに限らず
// 異常終了(クラッシュ)や、その他の予測不能な結果になる可能性もある。
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
