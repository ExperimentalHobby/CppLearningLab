# 31. TCPクライアント/サーバー

## 目的
ソケットAPIの基本（bind/listen/accept、connect/send/recv）を理解し、
TCPによるクライアント・サーバー間通信を実装する。通信課題全体の入り口。

## 学習ポイント
- TCPソケットの生成・接続・送受信・クローズ
- サーバー側の待受処理（bind/listen/accept）
- バイト列とアプリケーションデータの境界（メッセージフレーミングの必要性）

## 推奨ライブラリ/ツール
- Winsock2（Windows標準）、または Boost.Asio

本課題では**Winsock2**を採用する。開発環境にBoost.Asio(vcpkg等)が導入されて
おらず追加インストールもできないため、Windows SDK標準の`ws2_32.lib`のみで
構成する(詳細はルート [README.md](../README.md#通信31-38番台で採用したライブラリについて) を参照)。

## 成果物イメージ
サーバーが1接続を受け付け、クライアントから送った文字列をそのまま
返す（エコーサーバー）シンプルなプログラム一式。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 31_TCPClientServer ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug

# 別々のターミナルで
./out/build/x64-debug/TCPEchoServer [ポート番号(省略時12345)]
./out/build/x64-debug/TCPEchoClient <host> <port> <メッセージ>
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `WinsockGuard`(`winsock_guard.h`/`.cpp`) | `WSAStartup`/`WSACleanup`をRAIIで管理 | Winsock初期化・後始末 |
| `TcpListener`(`Listen`/`Accept`) | `socket`/`bind`/`listen`/`accept` | サーバー側の待受処理 |
| `TcpConnection`(`Connect`/`Send`/`Close`) | `socket`/`connect`/`send`/`closesocket` | TCPソケットの生成・接続・送受信・クローズ |
| `TcpConnection::ReceiveLine`(改行区切りのフレーミング) | 受信バッファを保持し`'\n'`が来るまで蓄積してから1メッセージとして返す | バイト列とアプリケーションデータの境界(メッセージフレーミングの必要性) |

`ReceiveLine`はTCPが「バイトの流れ」でしかなく、1回の`recv`が
必ずしも送信側の1回の`send`と対応しない(複数メッセージがまとめて届いたり、
1メッセージが分割されて届いたりしうる)ことへの対処例になっている。

## 動作確認

ビルド・実行し、以下を確認済み。

- サーバーを起動しクライアントから接続、送った文字列がそのままエコーされて
  返ること
- クライアントを2回連続で実行(別々の接続)しても、サーバーが順に
  受け入れて両方とも正しくエコーすること
