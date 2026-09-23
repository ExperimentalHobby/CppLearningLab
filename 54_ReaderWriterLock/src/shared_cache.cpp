#include "shared_cache.h"

namespace concurrency {

void SharedCache::Write(const std::string& key, int value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_[key] = value;
}

std::optional<int> SharedCache::Read(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    const auto it = data_.find(key);
    if (it == data_.end()) {
        return std::nullopt;
    }
    return it->second;
}

}  // namespace concurrency
