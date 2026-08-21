#ifndef SCOPED_TRACE_H
#define SCOPED_TRACE_H

#include <string>

// RAIIで「入った」「出た」をログに残すクラス。
// デストラクタはスコープを正常に抜けるときだけでなく、例外がスコープを通り抜けて
// 巻き戻る(スタックアンワインド)ときにも必ず呼ばれる。これにより、
// 途中で例外が発生しても後始末(ここではログ出力)が保証されることを確認できる。
class ScopedTrace {
public:
    explicit ScopedTrace(std::string label);
    ~ScopedTrace();

    ScopedTrace(const ScopedTrace&) = delete;
    ScopedTrace& operator=(const ScopedTrace&) = delete;

private:
    std::string label_;
};

#endif  // SCOPED_TRACE_H
