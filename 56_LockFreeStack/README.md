# 56. ロックフリースタック

## 目的
`std::atomic`の`compare_exchange`による、mutexを使わないロックフリースタック
(Treiber stack)を実装する。51番の基礎知識を応用した、並行処理の中でも難易度の
高い題材。

## 学習ポイント
- 単方向連結リストのノードを`std::atomic<Node*>`の先頭ポインタで管理する設計
- `Push()`/`Pop()`を`compare_exchange_weak`のリトライループで実装する
- ABA問題(ポインタの値が偶然一致してしまう問題)の説明と、この課題の範囲で
  どこまで対処するか(タグ付きポインタ等、学習用途としての簡略化)
- 複数スレッドから同時にPush/Popしても要素が失われない・壊れないことをテストで
  確認する

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<atomic>`)

## 設計上の簡略化(メモリ回収とABA問題)

**メモリ回収の簡略化**: `Pop()`で取り外したノードをその場で`delete`すると、
他スレッドがまだそのノードを参照中(CASリトライの途中でnext等を読んでいる)に
use-after-freeになりうる。本実装は教育目的の簡略化として、取り外したノードを
別のアトミックな連結リスト(`retiredHead_`)に退避するだけに留め、実際の解放は
デストラクタ(全スレッド合流後が前提)でまとめて行う。ハザードポインタや
エポックベース回収等の本格的な安全回収はスコープ外。

**ABA問題**: `compare_exchange`は「値(ポインタのアドレス)」だけを比較するため、
あるポインタ値が一度別の用途に使われた後、たまたま同じアドレスに戻ってきた場合に
誤って成功したと判定してしまう可能性がある。本実装は上記の遅延回収によりノードの
メモリを実際には再利用(alloc使い回し)しないためこのスコープでは実害が出にくいが、
タグ付きポインタ等による汎用的な解決は行っていない。

## 成果物イメージ
複数producer/複数consumerが同時にPush/PopするデモCLI。

## ビルド方法

```sh
# 56_LockFreeStack ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/LockFreeStack
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/lock_free_stack.h`(ヘッダーオンリーテンプレート) | `LockFreeStack<T>`。`Push`/`Pop`を`compare_exchange_weak`のリトライループで実装 | 全学習ポイント |
| `src/main.cpp` | 複数producer(4)/複数consumer(3)によるPush/Popデモ | 全体の統合デモ |

08番の`FixedStack`/09番の`BlockingQueue<T>`と同様、クラステンプレートのため
ヘッダーオンリーで実装している。

## 動作確認

- `test/lock_free_stack_test.cpp`(6テスト):
  - `PushThenPopReturnsSameValue`/`PopFromEmptyStackReturnsNullopt`/
    `MaintainsLifoOrderSingleThreaded`: 単一スレッドでの基本動作を確認。
  - `MultipleThreadsPushAllValuesWithoutLoss`: 8スレッド×2000回の`Push()`後、
    単一スレッドで全件`Pop()`し、件数と値が一致することを確認。`Push()`を
    `compare_exchange_weak`ではなく素朴な`load()`→`store()`に差し替えてテストを
    実行し、競合で176件のPushが失われる(Red、16000件中15824件しか届かない)
    ことを確認した上で、`compare_exchange_weak`のリトライループに戻して
    Green化した。
  - `PopRetriesInternallyWhenCasIsContended`: 2件Pushされたスタックに対し、
    2スレッドが同時にそれぞれ1回だけ`Pop()`を呼んでも、300回繰り返して
    毎回両方とも値を取得できることを確認。`Pop()`のCASが1回失敗しただけで
    リトライせず`std::nullopt`を返してしまう実装(競合を確実に再現するための
    一時的なsleep付き)に差し替えてテストを実行し、300回全てで片方が
    `nullopt`になる(Red)ことを確認した上で、`compare_exchange_weak`の
    リトライループに戻してGreen化した。
  - `ConcurrentPushAndPopDeliverAllValuesExactlyOnce`: 4producer×3consumerで、
    送信した8000件が重複・欠落なく届くことを確認(09番の`BlockingQueue`テストと
    同様のfetch_add予約パターンを使用)。
- `LockFreeStackTests.exe`実行でテスト6件全てパスすることを、デッドロック対策の
  タイムアウト監視付きで複数回(連続5回実行しても常にパスすること)確認。
- `LockFreeStack.exe`実行で、4producer×3consumerが送信した4000件全てが
  重複・欠落なく受信され、終了コード0で終わることを確認した。
