// 複数mutexの取得順序に起因するデッドロックと、その回避方法3種類。
#pragma once

#include <chrono>

#include "bank_account.h"

namespace concurrency {

// 危険: from→toの順に個別にロックを取得する素朴な実装。ロック取得の間に
// 意図的な小さいsleepを挟んでおり、A→B、B→Aの逆方向送金が同時に走ると、
// 「片方がAを、もう片方がBを持ったまま、互いに相手のロックを待ち続ける」
// 循環待機(デッドロック)を確実に再現できる。
void TransferNaive(BankAccount& from, BankAccount& to, int amount);

// 安全: std::scoped_lockで両方のロックを原子的に取得する(内部で
// デッドロック回避アルゴリズムを使うため、呼び出し順序に依存しない)。
void TransferSafe(BankAccount& from, BankAccount& to, int amount);

// 安全: try_lock_forでタイムアウト付きにロックを試み、2つ目が取れなければ
// 1つ目も手放してリトライする(デッドロックを検知して諦めるアプローチ)。
bool TransferWithTimeout(BankAccount& from, BankAccount& to, int amount,
                          std::chrono::milliseconds timeout);

}  // namespace concurrency
