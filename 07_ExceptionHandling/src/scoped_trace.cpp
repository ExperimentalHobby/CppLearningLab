#include "scoped_trace.h"

#include <iostream>
#include <utility>

ScopedTrace::ScopedTrace(std::string label) : label_(std::move(label)) {
    std::cout << "  [ScopedTrace] 開始: " << label_ << std::endl;
}

ScopedTrace::~ScopedTrace() {
    std::cout << "  [ScopedTrace] 終了: " << label_ << std::endl;
}
