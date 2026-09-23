// 58. 並列アルゴリズム(std::execution)
//
// C++17の<execution>実行ポリシーによる並列アルゴリズムを、
// seq/par/par_unseqの比較とデータ競合の実演で確認する。
#include <windows.h>

#include <chrono>
#include <execution>
#include <iostream>
#include <numeric>
#include <vector>

#include "parallel_algorithms.h"

namespace {

std::vector<int> MakeLargeData() {
    // 二乗の総和がlong longの範囲(約9.2e18)に収まるようにサイズを抑える
    // (200万件なら二乗和は約2.7e18で余裕がある。500万件だとオーバーフロー
    // していた)。
    std::vector<int> data(2'000'000);
    std::iota(data.begin(), data.end(), 0);
    return data;
}

// デモ1: SumOfSquaresをseq/par/par_unseqで実行し、結果が一致すること・
// 処理時間の傾向を確認する。
void RunSumOfSquaresDemo() {
    const std::vector<int> data = MakeLargeData();

    const auto run = [&](const char* label, auto&& policy) {
        const auto start = std::chrono::steady_clock::now();
        const long long result = concurrency::SumOfSquares(policy, data);
        const auto elapsedMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        std::cout << "[" << label << "] 結果=" << result << " 所要時間=" << elapsedMs << "ms\n";
    };

    run("seq", std::execution::seq);
    run("par", std::execution::par);
    run("par_unseq", std::execution::par_unseq);
}

// デモ2: 並列アルゴリズムに渡す関数が満たすべき制約(データ競合を起こさない
// こと)の実演。非atomicな共有カウンタをstd::for_each(par)でインクリメント
// すると、09番のUnsafeCounterと同様にlost updateが起きうる。
void RunDataRaceWarningDemo() {
    std::vector<int> data(1'000'000, 1);
    int unsafeCounter = 0;

    std::for_each(std::execution::par, data.begin(), data.end(), [&unsafeCounter](int) {
        // 危険: 非atomicな共有変数への書き込みを、並列実行される関数の中で
        // 行っている。複数スレッドから同時に++unsafeCounterされるとデータ
        // 競合になり、C++のメモリモデル上は未定義動作(undefined behavior)。
        ++unsafeCounter;
    });

    const int expected = static_cast<int>(data.size());
    std::cout << "[DataRaceWarning] 期待値=" << expected << " 実際の値=" << unsafeCounter;
    if (unsafeCounter != expected) {
        std::cout << " (データ競合により更新が失われました。並列アルゴリズムに渡す関数は"
                     "データ競合を起こさないこと(または適切な同期を取ること)が必須です)";
    } else {
        std::cout << " (たまたま一致しましたが、非atomicな共有変数への並列書き込みである"
                     "ことに変わりはなく、保証はありません)";
    }
    std::cout << "\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ1: SumOfSquaresをseq/par/par_unseqで比較 ===\n";
    RunSumOfSquaresDemo();

    std::cout << "\n=== デモ2: 並列アルゴリズムでのデータ競合の実演(注意喚起) ===\n";
    RunDataRaceWarningDemo();

    return 0;
}
