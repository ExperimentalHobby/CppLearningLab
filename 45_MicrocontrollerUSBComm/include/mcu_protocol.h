// PC⇔マイコン間の単純なテキストコマンド-応答プロトコル。
//
// コマンドは1行のASCII文字列("LED_ON"等)、応答も1行の文字列("OK"/"ERROR"/
// "SENSOR:<値>")という、Arduino等の非力なマイコンでも実装しやすい単純な
// 形式にしている。文字列の組み立て・解釈はSerialPort(通信層)に依存しない
// 純粋な処理であり、単体テストできる。
#pragma once

#include <optional>
#include <string>

namespace mcu {

enum class Command {
    kLedOn,
    kLedOff,
    kGetSensor,
};

// commandに対応する送信用の1行(末尾に'\n'付き)を組み立てる。
// 例: kLedOn -> "LED_ON\n"
std::string BuildCommandLine(Command command);

struct ResponseResult {
    // マイコン側が"OK"または"SENSOR:<値>"(値を取得できた)を返した場合true。
    // "ERROR"や解釈できない応答の場合はfalse。
    bool ok = false;

    // "SENSOR:<値>"応答から取得した数値。それ以外の応答ではstd::nullopt。
    std::optional<int> sensorValue;

    // 受信した応答行そのもの(改行は含まない)。ログ表示等に使う。
    std::string raw;
};

// マイコンからの応答行(改行を含まない1行)を解釈する。
// "OK" -> ok=true、"ERROR" -> ok=false、"SENSOR:123" -> ok=true・sensorValue=123、
// それ以外の解釈できない文字列は ok=false・sensorValue=nulloptとして扱う。
ResponseResult ParseResponse(const std::string& line);

}  // namespace mcu
