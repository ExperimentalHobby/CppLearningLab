// ファイル転送プロトコルのペイロード形式・チャンク分割・チェックサム計算。
//
// ネットワーク/ファイルI/Oに依存しない純粋なロジックとして実装しており、
// 単体テストできる。実際の送受信はbinary_protocol.h(Serialize/FrameParser)と
// USB通信層(43/44番)を組み合わせて行う想定。
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace filexfer {

class FileTransferError : public std::runtime_error {
   public:
    explicit FileTransferError(const std::string& message) : std::runtime_error(message) {}
};

// contentのSHA-256ハッシュ値を計算し、小文字16進数64桁の文字列で返す
// (37_WebSocketCommunicationのSha1関数と同じくWindows標準のCNG(BCrypt) APIを使う)。
std::string ComputeSha256Hex(const std::string& content);

// contentをchunkSizeバイトごとに分割する(最後のチャンクはchunkSizeに満たなくてよい)。
// contentが空の場合は空のvectorを返す。chunkSizeが0の場合は例外を投げる。
std::vector<std::string> SplitIntoChunks(const std::string& content, size_t chunkSize);

// kFileStartのペイロード形式: [2byte fileNameLen][fileName][8byte fileSize]
//                             [2byte checksumLen][checksumHex]
struct FileStartInfo {
    std::string fileName;
    uint64_t fileSize = 0;
    std::string checksumHex;
};
std::string EncodeFileStart(const std::string& fileName, uint64_t fileSize,
                             const std::string& checksumHex);
FileStartInfo DecodeFileStart(const std::string& payload);

// kFileChunkのペイロード形式: [4byte sequenceNumber][チャンクデータ(残り全部)]
struct FileChunkInfo {
    uint32_t sequenceNumber = 0;
    std::string data;
};
std::string EncodeFileChunk(uint32_t sequenceNumber, const std::string& data);
FileChunkInfo DecodeFileChunk(const std::string& payload);

// 受信したチャンク群をシーケンス番号の昇順に並べ替えてから結合し、元の
// ファイル内容を復元する。USB/TCP転送ではチャンクが送信順と異なる順序で
// 届く可能性があるため、受信順ではなくシーケンス番号でソートしてから結合する。
std::string ReassembleChunks(std::vector<FileChunkInfo> chunks);

}  // namespace filexfer
