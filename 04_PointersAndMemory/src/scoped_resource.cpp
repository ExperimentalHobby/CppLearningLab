#include "scoped_resource.h"

#include <iostream>
#include <utility>

ScopedResource::ScopedResource(std::string name) : name_(std::move(name)) {
    std::cout << "  [ScopedResource] 取得: " << name_ << std::endl;
}

ScopedResource::~ScopedResource() {
    std::cout << "  [ScopedResource] 解放: " << name_ << std::endl;
}
