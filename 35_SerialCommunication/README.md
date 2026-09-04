# 35. シリアル通信

## 目的
RS-232/COMポートを使ったシリアル通信の基本を理解し、
USB課題（41-47番）で扱う仮想COMポート通信の前提知識を身につける。

## 学習ポイント
- COMポートのオープン/クローズ、ボーレート等の通信パラメータ設定
- 同期/非同期読み書き
- 改行コードやパケット区切りを使ったメッセージフレーミング

## 推奨ライブラリ/ツール
- Windows API（`CreateFile`, `SetCommState`等）、または Boost.Asio のシリアルポート機能

本課題では**Windows API**を採用する(理由はルート
[README.md](../README.md#通信31-38番台で採用したライブラリについて) を参照)。

## 成果物イメージ
PC上で仮想COMポートペア（`com0com`等）を使い、片方に送信した文字列を
もう片方で受信・表示するプログラム。

本開発環境には`com0com`等の仮想COMポートペアも物理シリアルデバイスも
無いため、成果物イメージ通りの「片方に送信→もう片方で受信」は実演できない。
代わりに、本開発環境で実際に開ける2つのCOMポート(`COM1`・`COM4`、詳細は
「動作確認・環境上の制約」を参照)を使い、オープン/設定/送受信APIの呼び出し
自体が正しく機能することを確認する構成にした。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 35_SerialCommunication ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug

./out/build/x64-debug/SerialCommunication probe <ポート名> [ボーレート]
./out/build/x64-debug/SerialCommunication send <ポート名> <メッセージ> [ボーレート]
./out/build/x64-debug/SerialCommunication receive <ポート名> [タイムアウトms] [ボーレート]
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `SerialPort::Open`/`Close` | `CreateFileW`でポートを開き、`SetCommState`(DCB)でボーレート/データビット/パリティ/ストップビットを設定 | COMポートのオープン/クローズ、通信パラメータ設定 |
| `SerialPort::GetAppliedSettings` | `GetCommState`で実際に適用された設定を読み戻す(設定が本当に反映されたことの確認) | 通信パラメータ設定 |
| `SerialPort::SetReadTimeout`/`COMMTIMEOUTS` | `ReadIntervalTimeout=MAXDWORD`+`ReadTotalTimeoutConstant`の組み合わせで、データがあれば即座に、無ければ指定時間で打ち切る同期読み取り | 同期/非同期読み書き(のうち同期読み取りとタイムアウト制御) |
| `SerialPort::Write`/`Read` | `WriteFile`/`ReadFile`によるバイト列の送受信 | COMポートの読み書き |

`\\\\.\\COM<n>`形式のデバイスパスで開くことで、COM10以降の2桁番号でも
問題なくオープンできるようにしている(素の`"COM1"`形式は1桁番号専用)。

## 動作確認・環境上の制約

本開発環境を調査したところ、`COM1`・`COM4`という2つのCOMポートが実在し
オープン可能だった(仮想マシン側の提供によるものと見られる)。ただし相互に
接続されておらず(`com0com`のようなヌルモデムペアではなく、自己ループバックも
していない)、一方に書き込んだデータをもう一方で受信することはできない。

確認できた範囲:

- `probe COM1 115200`・`probe COM4 9600`いずれも正常にオープンでき、
  指定したボーレート/データビット8/パリティNone/ストップビット1が
  `GetCommState`で正しく読み戻せること
- 存在しないポート名(`COM99`)を指定すると、`CreateFileW`が
  `ERROR_FILE_NOT_FOUND`(エラーコード2)で失敗し、`FormatMessage`による
  日本語のエラーメッセージ(「指定されたファイルが見つかりません。」)が
  正しく表示されること
- `send COM1 "Hello Serial"`が書き込みエラー無く完了すること
- `receive COM1 800`が、何も受信できないまま約800ms(指定値)でタイムアウトし、
  ハングしないこと(`COMMTIMEOUTS`の設定が機能していることの確認)

実際のバイト列送受信(送った内容が相手に届くこと)は、`com0com`等の仮想
COMポートペアを導入するか、実機のシリアルケーブル/USBシリアル変換器を
2台のCOMポート間に接続した環境で改めて確認する必要がある。
