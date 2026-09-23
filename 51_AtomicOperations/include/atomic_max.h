// compare_exchange_weak()による「現在値より大きければ更新」の原子的な
// 実装。target.load()で読み、candidateの方が大きければtarget.store()する
// ……という素朴な実装は、loadしてからstoreするまでの間に別スレッドが
// 割り込める(TOCTOU)ため、複数スレッドが競合すると一部の更新が失われる。
// compare_exchange_weakは「読んだ時の値のままなら書き込む、変わっていたら
// 読み直してリトライする」を1つの原子操作で行うため、この競合が起きない。
#pragma once

#include <atomic>

namespace concurrency {

void UpdateMaxAtomic(std::atomic<int>& target, int candidate);

}  // namespace concurrency
