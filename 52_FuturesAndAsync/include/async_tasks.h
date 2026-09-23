// std::future/std::async/std::packaged_taskによる非同期タスクの実行例。
#pragma once

#include <future>
#include <thread>

namespace async_ops {

// std::async(std::launch::async, ...)でxの平方を計算するタスクを起動し、
// 結果を受け取るfutureを返す。
std::future<int> ComputeSquareAsync(int x);

// 指定したlaunch policyでタスクを実行し、そのタスクが実行されたスレッドの
// thread::idを返すfutureを返す。
//
// std::launch::deferredを指定した場合、タスクは即座には実行されず、
// 呼び出し元が返り値のfutureに対してget()/wait()を呼んだ時点で「呼び出し元
// 自身のスレッド上で」遅延実行される。そのためget()の戻り値は呼び出し元の
// thread::idと一致する。
//
// std::launch::asyncを指定した場合、呼び出し時点で即座に別スレッド上で
// 実行が始まるため、get()の戻り値は呼び出し元のthread::idとは異なる。
std::future<std::thread::id> GetExecutionThreadId(std::launch policy);

// std::asyncで実行した先で例外を投げるタスク。future::get()側で例外が
// 再送出されることを確認するために使う。
std::future<int> ThrowingTaskAsync();

// std::packaged_taskで「実行するタスク」と「結果の受け取り(future)」を
// 分離する例。packaged_taskを別スレッドに渡してa+bを計算する。
std::future<int> RunAddViaPackagedTask(int a, int b);

}  // namespace async_ops
