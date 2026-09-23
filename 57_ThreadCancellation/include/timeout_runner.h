// 長時間実行中のタスクに対してタイムアウトを設定し、時間内に終わらなければ
// キャンセルする仕組み。
#pragma once

#include <chrono>

#include "cancellable_task.h"

namespace concurrency {

// taskを起動し、timeout以内に終了すればtrueを返す。timeout以内に終わらなければ
// RequestCancel()してキャンセルし、falseを返す。
bool RunWithTimeout(CancellableTask& task, std::chrono::milliseconds timeout);

}  // namespace concurrency
