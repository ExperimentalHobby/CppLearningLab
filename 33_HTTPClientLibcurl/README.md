# 33. HTTPクライアント

## 目的
既存の高レベルプロトコル（HTTP）をライブラリ経由で利用し、
Web APIの呼び出しができるようになる。

## 学習ポイント
- HTTPリクエスト/レスポンスの基本（メソッド、ヘッダー、ボディ、ステータスコード）
- libcurl等のライブラリを使ったGET/POSTリクエストの実装
- JSONレスポンスのパース（軽量JSONライブラリの利用）

## 推奨ライブラリ/ツール
- libcurl、(任意) nlohmann/json

libcurl/nlohmann/jsonのいずれも本開発環境に無く追加インストールもできないため、
本課題では**WinHTTP**(Windows標準の高レベルHTTPクライアントAPI、`winhttp.lib`、
HTTPS対応、追加インストール不要)と、自作の最小限JSONパーサーを使う
(詳細はルート [README.md](../README.md#通信31-38番台で採用したライブラリについて) を参照)。

## 成果物イメージ
公開APIなど任意のHTTP APIにGETリクエストを送り、レスポンスのJSONから
必要な情報を取り出して表示するコマンドラインツール。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 33_HTTPClientLibcurl ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/HTTPClient [GitHub APIのパス(省略時 /repos/octocat/Hello-World)]
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `http::HttpsGet`(`http_client.h`/`.cpp`) | `WinHttpOpen`→`Connect`→`OpenRequest`→`SendRequest`→`ReceiveResponse`→`ReadData`の一連の呼び出し。ステータスコードとボディを返す | HTTPリクエスト/レスポンスの基本、ライブラリを使ったGETリクエストの実装 |
| `json::ParseJson`/`JsonValue`(`json_value.h`/`.cpp`) | オブジェクト/配列/文字列(エスケープ含む)/数値/真偽値/nullをサポートする再帰下降パーサー | JSONレスポンスのパース |
| `src/main.cpp` | GitHub APIから取得したJSONの`full_name`/`description`/`stargazers_count`等を`JsonValue::Find`で取り出して表示 | Web APIの呼び出し、JSONからの必要な情報の取り出し |

## 動作確認

- 実際に`https://api.github.com/repos/octocat/Hello-World`にGETリクエストを
  送信し、ステータスコード200と`full_name`/`description`/`stargazers_count`
  等の期待するフィールドが取得できることを確認した
  (本開発環境から外部HTTPS通信が可能であることも確認済み)
- 存在しないリポジトリへのパスを指定すると、GitHub APIからの404レスポンスと
  エラーメッセージ本文がそのまま表示されることを確認した
- `JsonValue`単体は、ネスト・配列・真偽値・null・エスケープされた引用符や
  日本語を含む文字列を含むテストケースを一時的な検証用プログラム
  (リポジトリには含めていない)で確認し、存在しないフィールドへの
  `Find`が例外を投げずnullptrを返すこと、構文エラーのJSONで
  `JsonParseError`が投げられることも確認した
