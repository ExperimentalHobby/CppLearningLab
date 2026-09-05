// 47. 独自ファイル転送プロトコル
//
// 38番のバイナリプロトコル(ヘッダー+ペイロード形式、FrameParserによる
// ストリーミング再構成)の考え方をファイル転送に応用したデモ。
//
// 注意: 本開発環境にはUSB経由で接続できる実機マイコンが無いため、実際の
// USBデバイス越しの転送は未確認。代わりに、送信側でSerializeしたバイト列を
// そのまま受信側のFrameParserへ給餌することで、プロトコル自体の正しさ
// (チャンク分割→送信→受信→再構成→チェックサム照合)を自己完結で確認する
// デモになっている(詳細はREADMEの「動作確認」を参照)。
#include <windows.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "binary_protocol.h"
#include "file_transfer.h"

namespace {

constexpr size_t kChunkSize = 256;  // 実機のUSB CDC等ではより小さい値が現実的

std::string ReadFileContent(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw filexfer::FileTransferError("ファイルを開けませんでした: " + path);
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

// 送信側: ファイルをkFileStart+複数のkFileChunk+kFileEndに組み立て、
// Serializeした結果を1本のバイト列として返す(実機ではこれをUSB経由で送る)。
std::string BuildTransferStream(const std::string& fileName, const std::string& content) {
    const std::string checksum = filexfer::ComputeSha256Hex(content);
    const auto chunks = filexfer::SplitIntoChunks(content, kChunkSize);

    std::string stream;
    proto::Message startMessage;
    startMessage.command = proto::Command::kFileStart;
    startMessage.payload = filexfer::EncodeFileStart(fileName, content.size(), checksum);
    stream += proto::Serialize(startMessage);

    for (size_t i = 0; i < chunks.size(); ++i) {
        proto::Message chunkMessage;
        chunkMessage.command = proto::Command::kFileChunk;
        chunkMessage.payload = filexfer::EncodeFileChunk(static_cast<uint32_t>(i), chunks[i]);
        stream += proto::Serialize(chunkMessage);
    }

    proto::Message endMessage;
    endMessage.command = proto::Command::kFileEnd;
    stream += proto::Serialize(endMessage);
    return stream;
}

}  // namespace

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    if (argc < 2) {
        std::cout << "使い方: " << argv[0] << " <転送するファイルのパス>\n";
        return 1;
    }

    try {
        const std::string path = argv[1];
        const std::string content = ReadFileContent(path);
        const std::string originalChecksum = filexfer::ComputeSha256Hex(content);

        std::cout << "送信側: " << path << " (" << content.size() << "バイト) を"
                  << kChunkSize << "バイトずつのチャンクに分割して送信します。\n";
        std::cout << "送信側チェックサム(SHA-256): " << originalChecksum << "\n";

        const std::string stream = BuildTransferStream(path, content);

        // 受信側: FrameParserでストリームを解析し、kFileStart/kFileChunk/kFileEndを
        // 順に受け取ってファイルを再構成する。
        std::vector<filexfer::FileChunkInfo> receivedChunks;
        filexfer::FileStartInfo startInfo;
        bool transferEnded = false;

        proto::FrameParser parser([&](const proto::Message& message) {
            switch (message.command) {
                case proto::Command::kFileStart:
                    startInfo = filexfer::DecodeFileStart(message.payload);
                    std::cout << "受信側: 転送開始通知(ファイル名=" << startInfo.fileName
                              << ", サイズ=" << startInfo.fileSize << "バイト)\n";
                    break;
                case proto::Command::kFileChunk:
                    receivedChunks.push_back(filexfer::DecodeFileChunk(message.payload));
                    break;
                case proto::Command::kFileEnd:
                    transferEnded = true;
                    break;
                default:
                    break;
            }
        });
        // 実機のUSB通信では1回の受信が細切れになることが多いが、ここでは
        // ストリーム全体を一度にFeedしても、38番同様に複数メッセージが
        // 正しく1つずつ取り出せることを確認する目的で一括で渡している。
        parser.Feed(stream);

        const std::string reassembled = filexfer::ReassembleChunks(std::move(receivedChunks));
        const std::string reassembledChecksum = filexfer::ComputeSha256Hex(reassembled);

        std::cout << "受信側: 転送終了通知=" << (transferEnded ? "受信済み" : "未受信") << "\n";
        std::cout << "受信側で再構成した内容のチェックサム: " << reassembledChecksum << "\n";
        std::cout << "通知されたチェックサムとの一致: "
                   << (startInfo.checksumHex == reassembledChecksum ? "OK" : "NG") << "\n";
        std::cout << "元ファイルとの内容一致: " << (reassembled == content ? "OK" : "NG") << "\n";
    } catch (const filexfer::FileTransferError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    } catch (const proto::ProtocolError& e) {
        std::cerr << "プロトコルエラー: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
