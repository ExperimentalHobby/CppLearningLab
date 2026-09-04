# 38. 独自バイナリプロトコル

## 目的
テキストベースではなく、独自のバイナリフォーマットで通信プロトコルを設計し、
シリアライズ/デシリアライズ処理を実装する。USB課題（47番）にもつながる。

## 学習ポイント
- バイト列レイアウトの設計（ヘッダー、長さフィールド、ペイロード）
- エンディアン（バイトオーダー）の考慮
- 部分受信（TCPのストリーム特性）に対応したパース処理（受信バッファリング）

## 推奨ライブラリ/ツール
- 標準ライブラリのみ、31番のTCP実装の上に構築

## 成果物イメージ
「コマンドID + データ長 + ペイロード」形式の独自プロトコルで、
複数種類のコマンドをやり取りできるクライアント/サーバー。

## プロトコル設計

固定長ヘッダー(12バイト、全てネットワークバイトオーダー)+可変長ペイロード。

```
+----------------+---------+---------+----------+----------+
| magic (4bytes) | version | command | reserved | length   |
| "MYPB"         | 1 byte  | 1 byte  | 2 bytes  | 4 bytes  |
+----------------+---------+---------+----------+----------+
続けてlengthバイトのペイロード
```

コマンド一覧:

| コマンド | ペイロード | 用途 |
|---|---|---|
| `kPing`(1) | 無し | 疎通確認 |
| `kPong`(2) | 無し | `kPing`への応答 |
| `kEcho`(3) | 任意のバイト列 | そのまま送り返される |
| `kSetValue`(4) | `[2byte keyLen][key][2byte valueLen][value]` | 簡易キーバリューストアへの登録 |
| `kGetValue`(5) | `[2byte keyLen][key]` | 値の取得要求 |
| `kValueResult`(6) | `[1byte found][2byte valueLen][value]` | `kSetValue`/`kGetValue`への応答 |

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 38_BinaryProtocolDesign ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug

# 別々のターミナルで
./out/build/x64-debug/BinaryProtocolServer [ポート番号(省略時12349)]
./out/build/x64-debug/BinaryProtocolClient <host> <port>
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `proto::Serialize`/ヘッダーレイアウト | magic+version+command+reserved+lengthの固定12バイトヘッダー | バイト列レイアウトの設計(ヘッダー、長さフィールド、ペイロード) |
| `htonl`/`htons`(ヘッダーのlength、キー・バリューの長さフィールド) | ホストのバイトオーダーに関わらず一貫した形式で送受信する | エンディアン(バイトオーダー)の考慮 |
| `proto::FrameParser::Feed`/`ExtractCompleteFrames` | 受信バイト列を蓄積し、ヘッダー+ペイロードが揃うたびに1メッセージを取り出すストリーミングパーサー | 部分受信(TCPのストリーム特性)に対応したパース処理(受信バッファリング) |
| `EncodeKeyValue`/`DecodeKeyValue`等 | 文字列2つを1つのペイロードに詰める、より構造化されたレイアウトの例 | バイト列レイアウトの設計 |

`net::TcpConnection::ReceiveSome`(本課題で追加)は、1回の`recv()`で得られた
分だけをそのまま返す低レベルAPIで、「送信側が1回で送ったデータが、必ずしも
1回の`recv()`にまとまって届くとは限らない」というTCPの性質をそのまま
`FrameParser`に体験させるために使っている(31番の`ReceiveExact`のように
内部で複数回`recv()`して待ち合わせてしまうと、部分受信への対応という
学習ポイントが実装の外から見えなくなってしまうため、あえて分けている)。

## 動作確認

- `FrameParser`単体を一時的な検証用プログラム(リポジトリには含めていない)で
  以下のパターンで検証した(全てPASS)。
  - メッセージを**1バイトずつ**`Feed()`しても最終的に正しく1件復元されること
  - **複数メッセージ**(Ping+Echo+GetValue)が1回の`Feed()`にまとまって
    届いても、3件とも正しく取り出せること
  - 1つのメッセージの**ヘッダー途中・ペイロード途中**で分割されて複数回に
    分けて届いても正しく復元できること
  - 不正なマジックバイトを与えると`ProtocolError`が投げられること
- 実際にサーバー/クライアントを起動し、Ping→Pong、Echoのオウム返し、
  `SetValue("name","Alice")`→`GetValue("name")`で"Alice"が取得できること、
  存在しないキーの`GetValue`が`found=false`を返すことを確認した
