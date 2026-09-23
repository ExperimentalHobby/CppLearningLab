#include "parallel_algorithms.h"

#include <gtest/gtest.h>

#include <execution>
#include <numeric>
#include <vector>

using namespace concurrency;

namespace {

std::vector<int> MakeSampleData() {
    std::vector<int> data(200);
    std::iota(data.begin(), data.end(), -100);  // -100 ... 99
    return data;
}

}  // namespace

class SumOfSquaresTest : public ::testing::TestWithParam<int> {};

// ポリシーが違っても結果は一致するべき、という並列アルゴリズムの基本契約を
// 検証する。0=seq, 1=par, 2=par_unseq。
TEST_P(SumOfSquaresTest, MatchesSequentialBaseline) {
    const std::vector<int> data = MakeSampleData();
    long long expected = 0;
    for (int x : data) {
        expected += static_cast<long long>(x) * x;
    }

    long long actual = 0;
    switch (GetParam()) {
        case 0:
            actual = SumOfSquares(std::execution::seq, data);
            break;
        case 1:
            actual = SumOfSquares(std::execution::par, data);
            break;
        case 2:
            actual = SumOfSquares(std::execution::par_unseq, data);
            break;
    }

    EXPECT_EQ(actual, expected);
}

INSTANTIATE_TEST_SUITE_P(Policies, SumOfSquaresTest, ::testing::Values(0, 1, 2));

class SortAscendingTest : public ::testing::TestWithParam<int> {};

TEST_P(SortAscendingTest, SortsInAscendingOrder) {
    std::vector<int> data = MakeSampleData();
    std::vector<int> expected = data;
    std::sort(expected.begin(), expected.end());

    switch (GetParam()) {
        case 0:
            SortAscending(std::execution::seq, data);
            break;
        case 1:
            SortAscending(std::execution::par, data);
            break;
        case 2:
            SortAscending(std::execution::par_unseq, data);
            break;
    }

    EXPECT_EQ(data, expected);
}

INSTANTIATE_TEST_SUITE_P(Policies, SortAscendingTest, ::testing::Values(0, 1, 2));

class IncrementAllTest : public ::testing::TestWithParam<int> {};

TEST_P(IncrementAllTest, AddsOneToEveryElement) {
    std::vector<int> data = MakeSampleData();
    std::vector<int> expected = data;
    for (int& x : expected) {
        ++x;
    }

    switch (GetParam()) {
        case 0:
            IncrementAll(std::execution::seq, data);
            break;
        case 1:
            IncrementAll(std::execution::par, data);
            break;
        case 2:
            IncrementAll(std::execution::par_unseq, data);
            break;
    }

    EXPECT_EQ(data, expected);
}

INSTANTIATE_TEST_SUITE_P(Policies, IncrementAllTest, ::testing::Values(0, 1, 2));
