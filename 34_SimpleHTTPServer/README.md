# 34. 簡易HTTPサーバー

## 目的
HTTPプロトコルを自作のTCPサーバー上に実装し、リクエストのパースと
レスポンス生成の仕組みを理解する。

## 学習ポイント
- 31番のTCPサーバーを拡張し、HTTPリクエストライン/ヘッダーをパースする
- 簡単なルーティング（パスに応じて異なるレスポンスを返す）
- 静的ファイルの配信、適切なステータスコード/Content-Typeの付与

## 推奨ライブラリ/ツール
- 標準のソケットAPI（Winsock2）のみで自作、または軽量HTTPライブラリでの比較実装

本課題では31番と同じ**Winsock2**の上に、HTTPリクエストのパースとレスポンス
生成を完全に自作する。

## 成果物イメージ
ブラウザからアクセスすると簡単なHTMLページを返す、自作の最小HTTPサーバー。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 34_SimpleHTTPServer ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/SimpleHTTPServer [ポート番号(省略時8080)]
```

起動後、ブラウザまたは`curl`で`http://127.0.0.1:8080/`にアクセスする。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `httpsrv::ReadHttpRequest`(`http_message.h`/`.cpp`) | `TcpConnection::ReceiveLine`(31番)でリクエストライン+ヘッダー行を読み、メソッド/パス/バージョン/ヘッダーに分解する | 31番のTCPサーバーを拡張し、HTTPリクエストライン/ヘッダーをパースする |
| `httpsrv::HttpServer::AddRoute`/`Dispatch` | `"GET /path"`をキーにハンドラを登録・検索するルーティングテーブル | 簡単なルーティング(パスに応じて異なるレスポンスを返す) |
| `httpsrv::BuildHttpResponse` | ステータスライン+`Content-Type`/`Content-Length`ヘッダー+ボディの組み立て | 適切なステータスコード/Content-Typeの付与 |
| `HttpServer::HandleConnection`(`std::thread`+`detach`) | 接続ごとにスレッドを立てて処理する(36番のマルチクライアント処理と同じ考え方) | (36番の前提となる複数接続処理の先取り) |

以下の制約はスコープを絞るために意図的に対応していない(README上の明記):

- **GETのみ対応**。リクエストボディ(POST等のペイロード)は読み取らない
- **静的ファイル配信は行わない**。ルートハンドラ内で文字列としてHTMLを返す
  (成果物イメージの「静的ファイルの配信」は簡略化している)
- **Keep-Alive非対応**。全レスポンスに`Connection: close`を付け、
  1リクエストごとに接続を閉じる

## 動作確認

ビルド・実行し、本開発環境で利用可能な`curl`から実際にHTTPリクエストを送って
確認済み。

```
$ curl -D - http://127.0.0.1:8080/
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
...

$ curl -D - http://127.0.0.1:8080/about
HTTP/1.1 200 OK
...

$ curl -D - http://127.0.0.1:8080/nonexistent
HTTP/1.1 404 Not Found
Content-Type: text/plain; charset=utf-8
...
```

- `/`と`/about`がそれぞれ200と対応するHTMLを、未定義パスが404を返すこと
- `Content-Type`/`Content-Length`が実際のボディと整合していること
- 5並列でリクエストを送っても全て200が返ること(接続ごとにスレッドを
  立てて処理しているため、1つの接続処理が他をブロックしない)を確認
