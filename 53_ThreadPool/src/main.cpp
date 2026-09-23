// 53. スレッドプール
//
// 固定数のワーカースレッドとタスクキューによるThreadPoolのデモ。
#include <windows.h>

#include <iostream>
#include <vector>

#include "thread_pool.h"

namespace {

void RunThreadPoolDemo() {
    async_ops::ThreadPool pool(4);

    std::vector<std::future<int>> results;
    constexpr int kTaskCount = 10;
    for (int i = 0; i < kTaskCount; ++i) {
        results.push_back(pool.Enqueue(
            [](int x) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                return x * x;
            },
            i));
    }

    for (int i = 0; i < kTaskCount; ++i) {
        std::cout << "[ThreadPool] " << i << "^2 = " << results[static_cast<size_t>(i)].get() << "\n";
    }

    pool.Shutdown();
    std::cout << "[ThreadPool] 全" << kTaskCount << "件のタスクが完了しました\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ: ThreadPool(4ワーカー)による並行タスク処理 ===\n";
    RunThreadPoolDemo();

    return 0;
}
