# 32. UDP通信

## 目的
コネクションレスなUDP通信の基本を理解し、TCPとの違い（信頼性、順序保証の
有無）を実装を通して体感する。

## 学習ポイント
- UDPソケットの生成、`sendto`/`recvfrom`
- コネクションレス通信の特性（パケットロスト、順序不同の可能性）
- ブロードキャスト/マルチキャストの基礎（任意）

## 推奨ライブラリ/ツール
- Winsock2、または Boost.Asio

本課題では31番と同じ**Winsock2**を採用する(理由はルート
[README.md](../README.md#通信31-38番台で採用したライブラリについて) を参照)。

## 成果物イメージ
簡単なUDPベースのメッセージ送受信ツール。意図的にパケットを送る間隔や
サイズを変え、TCP版(31番)との違いを観察する。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 32_UDPCommunication ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug

# 別々のターミナルで
./out/build/x64-debug/UDPEchoServer [ポート番号(省略時12346)]
./out/build/x64-debug/UDPClient <host> <port> <送信回数> [メッセージ接頭辞]
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `UdpSocket::SendTo`/`ReceiveFrom` | `sendto`/`recvfrom`。`connect`を呼ばずソケット生成後すぐ送受信できる | UDPソケットの生成、`sendto`/`recvfrom` |
| `UdpSocket::SetReceiveTimeout`/`UdpTimeoutError` | 応答が一定時間届かない場合をタイムアウトとして検知する | コネクションレス通信の特性(到達保証が無いことへの対処が呼び出し側の責務になる) |
| `SIO_UDP_CONNRESET`の無効化(`UdpSocket`コンストラクタ) | Windows固有の挙動の回避(後述) | コネクションレス通信の特性 |

### Windows固有の落とし穴: `WSAECONNRESET`

UDPは本来「送りっぱなし」のはずだが、Windowsでは「以前`sendto`した宛先から
ICMP Port Unreachableが返ってきていた」場合、次の`recvfrom`が
`WSAECONNRESET`(10054)で失敗するというTCPライクな挙動をする
(他のOSには無いWindows特有の仕様)。これを`SIO_UDP_CONNRESET`の無効化
(`UdpSocket`のコンストラクタで実施)で無効化しないと、応答が無い相手に
送った際に「タイムアウト」ではなく「謎のエラー」になってしまい、UDPの
性質と食い違う見え方になる。実際にこの問題を開発中に踏み抜き、
修正した(詳細は`src/udp_socket.cpp`のコメントを参照)。

## 動作確認

ビルド・実行し、以下を確認済み。

- サーバー起動後、クライアントから5回連続でメッセージを送信し、全て正しく
  エコーされること
- サーバーが存在しない(何も待ち受けていない)ポートへ送信すると、
  約2秒後に「応答がタイムアウトしました」と表示されハングしないこと
  (`SIO_UDP_CONNRESET`無効化により、`WSAECONNRESET`ではなく意図通り
  タイムアウトとして扱われることも確認した)
- サーバー起動直後(バインド完了前)にクライアントから送信すると、その1回だけ
  応答が届かずタイムアウトになることがある一方、TCP版(31番)では`connect`が
  接続確立まで待つためこのような現象が起きない。これはTCPが接続の確立を
  保証してから通信を始めるのに対し、UDPには「相手が受信できる状態になって
  いるか」を確認する仕組みが無いという違いをそのまま表している
