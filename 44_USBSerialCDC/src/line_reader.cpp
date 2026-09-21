#include "line_reader.h"

namespace serialcli {

namespace {
constexpr size_t kMaxLineLength = 4096;
}  // namespace

std::string ReadLine(const std::function<std::string()>& readChunk, std::string& pendingBuffer) {
    for (;;) {
        const size_t newlinePos = pendingBuffer.find('\n');
        if (newlinePos != std::string::npos) {
            std::string line = pendingBuffer.substr(0, newlinePos + 1);
            pendingBuffer.erase(0, newlinePos + 1);
            return line;
        }
        if (pendingBuffer.size() > kMaxLineLength) {
            throw ReadLineError(
                "受信データが上限(" + std::to_string(kMaxLineLength) +
                "バイト)を超えても改行が見つかりませんでした。"
                "デバイスとの同期が失われた可能性があります。");
        }
        const std::string chunk = readChunk();
        if (chunk.empty()) {
            // これ以上データが来ない(タイムアウト)。ここまでの内容(改行なし)を返す。
            std::string line = std::move(pendingBuffer);
            pendingBuffer.clear();
            return line;
        }
        pendingBuffer += chunk;
    }
}

}  // namespace serialcli
