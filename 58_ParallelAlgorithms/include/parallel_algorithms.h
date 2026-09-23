// C++17の<execution>実行ポリシー(std::execution::seq/par/par_unseq)を
// テンプレート引数として受け取り、標準アルゴリズムに渡す薄いラッパー群。
// テンプレートのためヘッダーオンリーで実装する(08番のFixedStack等と同様)。
#pragma once

#include <algorithm>
#include <execution>
#include <functional>
#include <numeric>
#include <vector>

namespace concurrency {

// 各要素を二乗した上での総和を求める。
template <typename ExecutionPolicy>
long long SumOfSquares(ExecutionPolicy&& policy, const std::vector<int>& data) {
    return std::transform_reduce(std::forward<ExecutionPolicy>(policy), data.begin(), data.end(), 0LL,
                                  std::plus<>(),
                                  [](int x) { return static_cast<long long>(x) * x; });
}

// 昇順にソートする。
template <typename ExecutionPolicy>
void SortAscending(ExecutionPolicy&& policy, std::vector<int>& data) {
    std::sort(std::forward<ExecutionPolicy>(policy), data.begin(), data.end());
}

// 全要素に1加算する。
template <typename ExecutionPolicy>
void IncrementAll(ExecutionPolicy&& policy, std::vector<int>& data) {
    std::for_each(std::forward<ExecutionPolicy>(policy), data.begin(), data.end(), [](int& x) { ++x; });
}

}  // namespace concurrency
