// シリアル通信の応答を「改行までの1行」として組み立てるロジック。
//
// SetReadTimeout()が設定するReadIntervalTimeout=MAXDWORD+
// ReadTotalTimeoutConstant>0という組み合わせ(serial_port.cpp参照)は
// 「1バイトでも受信済みならすぐ返す」特殊な挙動になるため、1回の読み取り
// 呼び出しだけでは改行までの1行分が揃っている保証が無い。実際のシリアル
// ポートに依存せず単体テストできるよう、データの読み取り自体はコールバック
// (readChunk)として受け取る。
#pragma once

#include <functional>
#include <stdexcept>
#include <string>

namespace serialcli {

class ReadLineError : public std::runtime_error {
   public:
    explicit ReadLineError(const std::string& message) : std::runtime_error(message) {}
};

// readChunk()を繰り返し呼び出し、改行に到達するか、readChunkが空文字列
// (これ以上データが来ない=タイムアウト)を返すまでデータを蓄積して
// 1行(改行込み、タイムアウト時は改行無し)を返す。
//
// readChunkが1回で複数行分のデータを返すことがある(例:"foo\nbar\n")。
// 最初の改行より後ろのデータを捨てると、そのデータが失われて以降の
// 呼び出しの対応がずれてしまうため、消費し切れなかった分は呼び出し側が
// 保持するpendingBufferに残し、次回の呼び出しで先に使う。
//
// 改行が来ないままデータが上限(4096バイト)を超えて届いた場合、受信
// ストリームの区切り位置を見失っている可能性が高いためReadLineErrorを
// 投げる。中途半端に打ち切ると、まだ届いていない残りのバイト列を次回
// 呼び出しが「次の行の先頭」と誤解釈し、それ以降の送受信がずれ続けて
// しまうため、復旧を試みずセッションを終了させる想定。
std::string ReadLine(const std::function<std::string()>& readChunk, std::string& pendingBuffer);

}  // namespace serialcli
