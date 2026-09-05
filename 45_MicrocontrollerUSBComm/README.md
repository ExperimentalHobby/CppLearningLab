# 45. マイコンとのUSB通信

## 目的
Arduino等の安価なマイコンボードとUSB経由でやり取りする実践的な課題を通じて、
ハードウェア連携の感覚をつかむ。

## 学習ポイント
- マイコン側のファームウェア(Arduino IDE等で作成する簡単なプログラム)とPC側C++
  アプリケーションの役割分担
- コマンド送信→マイコン側処理→応答受信、という一往復のプロトコル設計
- LED点灯やセンサー値取得など、実際に「モノが動く/読める」体験

## 採用ライブラリ/ツール
- 44番のUSB CDC通信コード(`serial_port.h`/`.cpp`)+ 独自のコマンド-応答
  プロトコル(`mcu_protocol.h`/`.cpp`)。

## 成果物イメージ
PCから"LED_ON"/"LED_OFF"のようなコマンドを送るとArduino上のLEDが
点灯/消灯し、センサー値を要求すると値が返ってくる簡単な制御アプリ。

**本開発環境にはArduino等の実機マイコンが接続されていないため、
実際のコマンド-応答のやり取りは未確認**(詳細は「動作確認」を参照)。

## プロトコル仕様

| コマンド(PC→マイコン) | 想定される応答(マイコン→PC) |
|---|---|
| `LED_ON` | `OK` |
| `LED_OFF` | `OK` |
| `GET_SENSOR` | `SENSOR:<整数値>` (例: `SENSOR:512`) |
| (異常時) | `ERROR` |

いずれも1行のASCII文字列(末尾`\n`)。Arduino側は`Serial.readStringUntil('\n')`
+`Serial.println()`程度の実装で対応できる想定。

## ビルド方法・実行方法

```sh
# 45_MicrocontrollerUSBComm ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/MicrocontrollerUSBComm <COMポート名> [ボーレート(既定9600)]
```

接続後、メニューから`1`(LED_ON)/`2`(LED_OFF)/`3`(GET_SENSOR)/`0`(終了)を選択する。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `BuildCommandLine()` | `Command`列挙値から送信用の1行を組み立てる | コマンド送信 |
| `ParseResponse()` | `"OK"`/`"ERROR"`/`"SENSOR:<値>"`を解釈する | 応答受信・一往復のプロトコル設計 |
| `main.cpp`のメニュー | 選択→送信→受信→表示のループ | PC側アプリケーションの役割 |

`BuildCommandLine`/`ParseResponse`はテキスト処理のみの純粋関数
(`SerialPort`に依存しない)であり、`test/`でGoogleTestによる単体テストを
行っている。

## 動作確認

- `test/mcu_protocol_test.cpp`のテスト(各コマンドの組み立て、OK/ERROR/
  SENSOR応答の解釈、負の値、不正な応答・未知の応答・空文字列の扱い)が
  全てパスすることを確認。
- 実機確認: 本開発環境にArduino等が接続されていないため、実際のコマンド-応答
  のやり取りは確認できない。35番のREADMEに記載されている実在するCOMポート
  (`COM1`)を使い、接続・`LED_ON`コマンドの送信がエラー無く完了し、応答が
  無い状態で正しくタイムアウトすることを確認した。

実際のマイコンとのやり取りは、Arduino等を接続できる環境で、上記の
プロトコル仕様に沿ったファームウェアを書き込んだ上で改めて確認する必要がある。
