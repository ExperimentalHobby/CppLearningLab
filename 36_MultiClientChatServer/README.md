# 36. マルチクライアントチャットサーバー

## 目的
複数クライアントを同時に処理するサーバー設計（マルチスレッドまたは非同期I/O）を
実装し、実用的なネットワークサーバーの構成を理解する。

## 学習ポイント
- 複数接続の同時処理（スレッドプール、または`select`/`epoll`相当の非同期I/O、
  もしくはBoost.Asioの非同期モデル）
- 共有状態（接続中クライアント一覧）への排他制御
- ブロードキャスト配信の実装

## 推奨ライブラリ/ツール
- `std::thread` + Winsock2、または Boost.Asio（非同期I/O）

本課題では**`std::thread` + Winsock2**を採用する(理由はルート
[README.md](../README.md#通信31-38番台で採用したライブラリについて) を参照)。

## 成果物イメージ
複数のクライアントが接続し、一人が送ったメッセージが全員に配信される
簡易チャットサーバー/クライアント。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 36_MultiClientChatServer ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug

# 別々のターミナルで
./out/build/x64-debug/ChatServer [ポート番号(省略時12347)]
./out/build/x64-debug/ChatClient <host> <port> <ニックネーム>
```

`ChatClient`は接続後、標準入力の行を送信し続ける対話的CLI。`quit`で終了する。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `ChatServer::Run`(接続ごとに`std::thread`を立てて`detach`) | クライアント数分だけスレッドを作る単純なモデル | 複数接続の同時処理 |
| `ChatServer::clients_`+`std::mutex mutex_` | 接続中クライアント一覧への排他アクセス | 共有状態(接続中クライアント一覧)への排他制御 |
| `ChatServer::Broadcast` | 送信者を除く全クライアントへ`SendLine` | ブロードキャスト配信の実装 |

`Broadcast`は`mutex_`を保持したまま全クライアントへの送信を行うため、
ブロードキャスト中は他の接続/切断処理と直列化される。パフォーマンスより
「共有状態への排他制御」という学習ポイントの分かりやすさを優先した設計。

## 動作確認

対話的な`ChatClient`とは別に、`ChatServer`のブロードキャストロジック自体を
生のTCP接続で直接検証した(標準入出力を介した対話コマンドの自動操作は
不安定なため、コアロジックを直接検証する方針にした、詳細は実装プランを参照)。
Alice/Bob/Carolの3クライアントを接続し、以下を確認済み。

- Bobが参加すると、先に接続していたAliceに「Bobが参加しました。」という
  通知が届くこと(参加した本人には届かない)
- Aliceが"Hello everyone"を送信すると、Bob・Carolの両方に
  「Alice: Hello everyone」が届き、**送信者自身には自分の発言が
  エコーバックされない**こと
- Bobが切断すると、残っているAlice・Carolに「Bobが退出しました。」という
  通知が届くこと
