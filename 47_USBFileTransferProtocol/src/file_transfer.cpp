#include "file_transfer.h"

// winsock2.hはwindows.hが内部でwinsock.h(旧API)を引き込む前に
// includeしないと、型の再定義で衝突する(Windows SDKの定番の罠)。
#include <winsock2.h>  // htonl/ntohl, htons/ntohs
#include <windows.h>

#include <bcrypt.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace filexfer {

namespace {

void AppendUint16(std::string& out, uint16_t value) {
    const uint16_t networkOrder = htons(value);
    out.append(reinterpret_cast<const char*>(&networkOrder), sizeof(networkOrder));
}

uint16_t ReadUint16(const std::string& data, size_t offset) {
    uint16_t networkOrder = 0;
    std::memcpy(&networkOrder, data.data() + offset, sizeof(networkOrder));
    return ntohs(networkOrder);
}

void AppendUint32(std::string& out, uint32_t value) {
    const uint32_t networkOrder = htonl(value);
    out.append(reinterpret_cast<const char*>(&networkOrder), sizeof(networkOrder));
}

uint32_t ReadUint32(const std::string& data, size_t offset) {
    uint32_t networkOrder = 0;
    std::memcpy(&networkOrder, data.data() + offset, sizeof(networkOrder));
    return ntohl(networkOrder);
}

void AppendUint64(std::string& out, uint64_t value) {
    // ネットワークバイトオーダーの64bit変換関数(htonll相当)はWinsock2に
    // 存在しないため、上位32bit/下位32bitに分けてhtonlで変換し連結する。
    const uint32_t high = static_cast<uint32_t>(value >> 32);
    const uint32_t low = static_cast<uint32_t>(value & 0xFFFFFFFFu);
    AppendUint32(out, high);
    AppendUint32(out, low);
}

uint64_t ReadUint64(const std::string& data, size_t offset) {
    const uint64_t high = ReadUint32(data, offset);
    const uint64_t low = ReadUint32(data, offset + 4);
    return (high << 32) | low;
}

void CheckUint16Length(const std::string& fieldName, size_t length) {
    if (length > (std::numeric_limits<uint16_t>::max)()) {
        throw FileTransferError(fieldName + "の長さがuint16_tの範囲を超えています: " +
                                std::to_string(length) + " bytes");
    }
}

}  // namespace

std::string ComputeSha256Hex(const std::string& content) {
    // BCryptHashDataの入力長はULONG(32bit)で受け取るため、content.size()
    // (64bit環境ではsize_t)がULONGの範囲(概ね4GiB)を超えると桁あふれし、
    // ちょうど4GiBの場合は長さ0として渡ってしまい、先頭が切り詰められた
    // 内容だけをハッシュ化した誤ったチェックサムを返してしまう。
    // 学習用途の転送デモでそこまで巨大な入力は想定しないため、事前に
    // 検証して拒否する。
    if (content.size() > (std::numeric_limits<ULONG>::max)()) {
        throw FileTransferError("contentのサイズがULONGの範囲を超えています: " +
                                std::to_string(content.size()) + " bytes");
    }

    BCRYPT_ALG_HANDLE algorithm = nullptr;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) {
        throw FileTransferError("BCryptOpenAlgorithmProviderに失敗しました");
    }

    DWORD hashObjectSize = 0;
    DWORD hashLength = 0;
    DWORD resultSize = 0;
    if (BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&hashObjectSize),
                           sizeof(hashObjectSize), &resultSize, 0) != 0 ||
        BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&hashLength),
                           sizeof(hashLength), &resultSize, 0) != 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        throw FileTransferError("BCryptGetPropertyに失敗しました");
    }

    std::vector<UCHAR> hashObject(hashObjectSize);
    BCRYPT_HASH_HANDLE hash = nullptr;
    if (BCryptCreateHash(algorithm, &hash, hashObject.data(), hashObjectSize, nullptr, 0, 0) != 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        throw FileTransferError("BCryptCreateHashに失敗しました");
    }

    if (BCryptHashData(hash, reinterpret_cast<PUCHAR>(const_cast<char*>(content.data())),
                        static_cast<ULONG>(content.size()), 0) != 0) {
        BCryptDestroyHash(hash);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        throw FileTransferError("BCryptHashDataに失敗しました");
    }

    std::vector<UCHAR> digest(hashLength);
    if (BCryptFinishHash(hash, digest.data(), hashLength, 0) != 0) {
        BCryptDestroyHash(hash);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        throw FileTransferError("BCryptFinishHashに失敗しました");
    }

    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);

    static const char kHexDigits[] = "0123456789abcdef";
    std::string hex;
    hex.reserve(digest.size() * 2);
    for (const UCHAR byte : digest) {
        hex += kHexDigits[(byte >> 4) & 0x0F];
        hex += kHexDigits[byte & 0x0F];
    }
    return hex;
}

std::vector<std::string> SplitIntoChunks(const std::string& content, size_t chunkSize) {
    if (chunkSize == 0) {
        throw FileTransferError("chunkSizeは1以上である必要があります");
    }
    std::vector<std::string> chunks;
    for (size_t offset = 0; offset < content.size(); offset += chunkSize) {
        chunks.push_back(content.substr(offset, chunkSize));
    }
    return chunks;
}

std::string EncodeFileStart(const std::string& fileName, uint64_t fileSize,
                             const std::string& checksumHex) {
    CheckUint16Length("fileName", fileName.size());
    CheckUint16Length("checksumHex", checksumHex.size());

    std::string payload;
    AppendUint16(payload, static_cast<uint16_t>(fileName.size()));
    payload += fileName;
    AppendUint64(payload, fileSize);
    AppendUint16(payload, static_cast<uint16_t>(checksumHex.size()));
    payload += checksumHex;
    return payload;
}

FileStartInfo DecodeFileStart(const std::string& payload) {
    if (payload.size() < 2) {
        throw FileTransferError("kFileStartペイロードが短すぎます");
    }
    const uint16_t fileNameLen = ReadUint16(payload, 0);
    size_t offset = 2;
    if (payload.size() < offset + fileNameLen + 8 + 2) {
        throw FileTransferError("kFileStartペイロードのfileName/fileSize部分が不正です");
    }

    FileStartInfo info;
    info.fileName = payload.substr(offset, fileNameLen);
    offset += fileNameLen;
    info.fileSize = ReadUint64(payload, offset);
    offset += 8;
    const uint16_t checksumLen = ReadUint16(payload, offset);
    offset += 2;
    if (payload.size() < offset + checksumLen) {
        throw FileTransferError("kFileStartペイロードのchecksum部分が不正です");
    }
    info.checksumHex = payload.substr(offset, checksumLen);
    return info;
}

std::string EncodeFileChunk(uint32_t sequenceNumber, const std::string& data) {
    std::string payload;
    AppendUint32(payload, sequenceNumber);
    payload += data;
    return payload;
}

FileChunkInfo DecodeFileChunk(const std::string& payload) {
    if (payload.size() < 4) {
        throw FileTransferError("kFileChunkペイロードが短すぎます");
    }
    FileChunkInfo info;
    info.sequenceNumber = ReadUint32(payload, 0);
    info.data = payload.substr(4);
    return info;
}

std::string ReassembleChunks(std::vector<FileChunkInfo> chunks) {
    std::sort(chunks.begin(), chunks.end(), [](const FileChunkInfo& a, const FileChunkInfo& b) {
        return a.sequenceNumber < b.sequenceNumber;
    });

    // ソートするだけでは、kAck/kNackによる再送を想定した場合に、重複した
    // シーケンス番号のチャンクが両方連結されてしまったり、欠落した
    // シーケンス番号があっても静かに受理されてしまう。0起点で連続して
    // いることを検証し、重複・欠落のどちらも例外として検出する。
    for (size_t i = 0; i < chunks.size(); ++i) {
        if (chunks[i].sequenceNumber != static_cast<uint32_t>(i)) {
            throw FileTransferError(
                "チャンクのシーケンス番号が連続していません(欠落または重複の可能性): 期待値=" +
                std::to_string(i) + " 実際=" + std::to_string(chunks[i].sequenceNumber));
        }
    }

    std::string result;
    for (const FileChunkInfo& chunk : chunks) {
        result += chunk.data;
    }
    return result;
}

}  // namespace filexfer
