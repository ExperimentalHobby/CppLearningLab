# 54. 読み書きロック(std::shared_mutex)

## 目的
`std::shared_mutex`による、複数の読み取りスレッドを同時に許可しつつ書き込み
スレッドとは排他する読み書きロックを学ぶ。09番の`std::mutex`(常に単一スレッド
のみ許可)との違いを体験する。

## 学習ポイント
- `std::shared_mutex`+`std::shared_lock`(読み取り)/`std::unique_lock`(書き込み)の
  使い分け
- 複数の読み取りスレッドが同時にデータへアクセスできることの確認
- 書き込み中は読み取り・書き込みどちらも待たされることの確認
- 単純な`std::mutex`で全アクセスを排他した場合との比較(読み取りが多い
  ワークロードでの効果を実感する)。09番の`ThreadSafeCounter`は`std::mutex`で
  読み取り(`Value()`)も含めて全て排他しており、`std::shared_mutex`ならその
  読み取り同士を並行させられる。

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<shared_mutex>`)

## 成果物イメージ
複数readerの同時アクセスと、writer実行中のreader排他を確認するデモCLI。

## ビルド方法

```sh
# 54_ReaderWriterLock ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/ReaderWriterLock
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/shared_cache.h`/`src/shared_cache.cpp` | `SharedCache`。`Write()`は`unique_lock`、`Read()`は`shared_lock`で保護したキーバリューストア。テスト用に`ReadWithLockHeld`/`WriteWithLockHeld`(ロックを保持した状態で任意の処理を実行するフック)も公開 | shared_lock/unique_lockの使い分け |
| `src/main.cpp` | ①複数readerの同時アクセス ②writer実行中のreader排他、の2デモ | 全体の統合デモ |

`ReadWithLockHeld`/`WriteWithLockHeld`は、44/45番の`ReadLine`で使ったコールバック
注入パターンと同様、ロックの粒度(どの区間が読み取り中/書き込み中なのか)を
外部のテストコードから観測できるようにするために用意している。

## 動作確認

- `test/shared_cache_test.cpp`(4テスト):
  - `WriteThenReadReturnsValue`/`ReadMissingKeyReturnsNullopt`: 基本動作を確認。
  - `MultipleReadersRunConcurrently`: 8個のreaderスレッドが同時にロック内へ入れる
    (最大同時人数が8と一致する)ことを確認。`ReadWithLockHeld`を`shared_lock`
    ではなく`unique_lock`で実装した状態でテストを実行し、readerが直列化されて
    最大同時人数が1のまま失敗する(Red)ことを確認した上で、`shared_lock`に
    戻してGreen化した。
  - `WriterExcludesReadersWhileWriting`: writerが排他ロックを保持している間、
    readerがそのタイミングでロックに入れない(書き込み中フラグがtrueの状態を
    観測しない)ことを確認。`WriteWithLockHeld`を`unique_lock`ではなく
    `shared_lock`で実装した状態でテストを実行し、readerが書き込み中に
    入り込めてしまう(8件中8件がtrueを観測、Red)ことを確認した上で、
    `unique_lock`に戻してGreen化した。
- `ReaderWriterLockTests.exe`実行でテスト4件全てパスすることを複数回
  (連続5回実行しても常にパスすること)確認。
- `ReaderWriterLock.exe`実行で、4人のreaderの読み取り開始ログが読み取り終了
  ログより先に全員分出力されること(同時実行の証拠)、writer実行中はreaderの
  読み取りが待たされ、writer終了後に初めてreaderが読み取れることを確認した。
  終了コード0で終わることも確認した。
