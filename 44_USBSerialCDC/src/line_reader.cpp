#include "line_reader.h"

namespace serialcli {

namespace {
constexpr size_t kMaxLineLength = 4096;
}  // namespace

std::string ReadLine(const std::function<std::string()>& readChunk, std::string& pendingBuffer) {
    for (;;) {
        const size_t newlinePos = pendingBuffer.find('\n');
        if (newlinePos != std::string::npos) {
            // find('\n')は改行の有無を最優先で見るため、「改行が見つからない
            // 場合のみ上限を見る」実装だと、改行を含む長大なチャンク
            // (例:5000バイトの改行なしデータの直後に'\n'が1つ来た場合)で
            // 上限チェックを回避できてしまう。改行が見つかった場合でも、
            // その1行自体の長さを上限と照合する(改行の後ろに次回以降処理する
            // 大きなデータが控えていても、今回返す行が短ければ問題ない)。
            if (newlinePos + 1 > kMaxLineLength) {
                throw ReadLineError(
                    "受信した1行が上限(" + std::to_string(kMaxLineLength) +
                    "バイト)を超えています。デバイスとの同期が失われた可能性があります。");
            }
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
