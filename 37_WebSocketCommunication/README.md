# 37. WebSocket通信

## 目的
HTTPのハンドシェイクから始まる双方向・リアルタイム通信プロトコルである
WebSocketを扱えるようになる。

## 学習ポイント
- WebSocketハンドシェイクの仕組み（HTTPからのアップグレード）
- フレームベースの双方向メッセージ送受信
- 36番のチャットサーバーとの比較（生TCP実装 vs 標準化プロトコル）

## 推奨ライブラリ/ツール
- Boost.Beast、または軽量WebSocketライブラリ（例: `websocketpp`）

Boost.Beast/websocketppが本開発環境に無いため、31番のWinsock2実装の上に
RFC 6455のハンドシェイク/フレーミングを自作する。SHA-1計算のみWindows標準の
**CNG(BCrypt) API**(`bcrypt.h`)を使う(理由はルート
[README.md](../README.md#通信31-38番台で採用したライブラリについて) を参照)。

## 成果物イメージ
36番のチャットサーバーをWebSocketベースで作り直し、ブラウザの
JavaScriptクライアントからも接続できることを確認する。

本課題では、スコープを絞って1対1のエコーサーバー/クライアント
(36番のような複数接続へのブロードキャストは行わない)として実装した。
ハンドシェイクとフレーミングというWebSocketの核となる仕組み自体の実装・
検証に焦点を当てている。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 37_WebSocketCommunication ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug

# 別々のターミナルで
./out/build/x64-debug/WebSocketServer [ポート番号(省略時12348)]
./out/build/x64-debug/WebSocketClient <host> <port> [メッセージ...]
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `ws::ComputeAcceptKey`(BCryptによるSHA-1+Base64) | `Sec-WebSocket-Key`+固定GUIDのSHA-1をBase64化して`Sec-WebSocket-Accept`を計算 | WebSocketハンドシェイクの仕組み(HTTPからのアップグレード) |
| `ws::PerformServerHandshake`/`PerformClientHandshake` | HTTPリクエスト/レスポンスの読み書きによるプロトコルアップグレード | HTTPからのアップグレード |
| `ws::SendTextFrame`/`ReceiveTextFrame` | FIN+opcode+ペイロード長(7/16/64bit)+マスク鍵を扱うフレームの送受信。クライアント→サーバーは必ずマスクし、サーバー→クライアントはマスクしない(RFC 6455) | フレームベースの双方向メッセージ送受信 |
| (36番との比較) | 36番は独自の改行区切りテキストプロトコル、37番は標準化されたバイナリフレーム形式 | 36番のチャットサーバーとの比較(生TCP実装 vs 標準化プロトコル) |

`net::TcpConnection::ReceiveExact`(31番の`tcp_socket`に本課題で追加)を使い、
改行に依存しないバイナリのフレーム境界を扱えるようにしている。

スコープの都合で以下は非対応(README上の明記):

- メッセージのフラグメンテーション(FIN=0の継続フレーム)
- close/ping/pong等の制御フレームの明示的な処理
- 拡張(permessage-deflate等)

## 動作確認

- **RFC 6455記載の既知のテストベクタ**(`Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==`
  → `Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=`)で`ComputeAcceptKey`が
  正確に一致することを一時的な検証用プログラム(リポジトリには含めていない)で確認した
- 実際にサーバー/クライアントを起動し、ハンドシェイク成立後、複数のテキスト
  メッセージ(日本語を含む)を送って`"echo: "`付きの応答が正しく返ることを確認した
- 125バイトを超える(16bit拡張長フィールドを使う)メッセージでも正しく
  往復することを確認した
