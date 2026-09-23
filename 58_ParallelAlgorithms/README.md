# 58. 並列アルゴリズム(std::execution)

## 目的
C++17の`<execution>`実行ポリシーによる並列アルゴリズムを学ぶ。51-57番で
手作りしてきた同期プリミティブに対し、標準ライブラリが提供する高レベルな
並列処理の抽象化を体験する、この帯の集大成。

## 学習ポイント
- `std::execution::seq`/`par`/`par_unseq`の違い
- `std::for_each`/`std::transform_reduce`/`std::sort`等を実行ポリシー付きで
  呼び出し、逐次実行との結果一致・処理時間の違いを確認する
- 並列アルゴリズムに渡す関数が満たすべき制約(データ競合を起こさないこと等)の
  説明

## 事前検証結果
Issueの起票時点では「MSVCの`<execution>`はConcurrency Runtimeにより標準
ライブラリへ組み込み実装されており、追加ライブラリ無しでビルドできる見込み
(実装時に要検証)」とされていた。実装開始時に最小プログラムで実際に検証し、
`std::execution::seq`/`par`/`par_unseq`を使う`std::transform_reduce`/
`std::sort`/`std::for_each`が、TBB等の追加ライブラリ無しで問題なく
ビルド・実行できることを確認した。

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<execution>`)。MSVCではConcurrency Runtimeによる
  組み込み実装のため追加ライブラリは不要(上記の事前検証結果を参照)。

## 成果物イメージ
`seq`/`par`/`par_unseq`の結果一致・処理時間比較と、データ競合の実演を行う
デモCLI。

## ビルド方法

```sh
# 58_ParallelAlgorithms ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/ParallelAlgorithms
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/parallel_algorithms.h`(ヘッダーオンリー、実行ポリシーがテンプレート引数のため) | `SumOfSquares`(`transform_reduce`)/`SortAscending`(`sort`)/`IncrementAll`(`for_each`)の3関数 | for_each/transform_reduce/sort等を実行ポリシー付きで呼び出す |
| `src/main.cpp` | ①3ポリシーでの結果一致・処理時間比較 ②データ競合の実演、の2デモ | seq/par/par_unseqの違い、データ競合の制約 |

## データ競合についての注意(重要な学習ポイント)

`src/main.cpp`の`RunDataRaceWarningDemo()`では、`std::for_each(std::execution::par, ...)`
に「非atomicな共有カウンタをインクリメントするだけ」の関数を渡している。
`par`ポリシーは複数スレッドから同時にこの関数を呼び出しうるため、09番の
`UnsafeCounter`と全く同じ理由でlost updateが起き、期待値(1,000,000)より
小さい値になる。**並列アルゴリズムに渡す関数は、データ競合を起こさないこと
(共有可変状態に触れない、または適切に同期すること)が呼び出し側の責任である**。
この制約を破ると、C++のメモリモデル上は未定義動作(undefined behavior)になる。

## 動作確認

- `test/parallel_algorithms_test.cpp`(9テスト、`seq`/`par`/`par_unseq`の
  3ポリシー×3関数をパラメータ化テストで検証):
  - `SumOfSquaresTest`: `SumOfSquares`を`x*x`ではなく`x`を返す(二乗し忘れ)
    実装に差し替えてテストを実行し、3ポリシー全てで不一致になる(Red)ことを
    確認した上で、`x*x`に戻してGreen化した。
  - `SortAscendingTest`: `SortAscending`を`std::greater<>`(降順)を使う実装に
    差し替えてテストを実行し、3ポリシー全てで不一致になる(Red)ことを確認した
    上で、昇順に戻してGreen化した。
  - `IncrementAllTest`: `IncrementAll`を`--x`(デクリメント)する実装に
    差し替えてテストを実行し、3ポリシー全てで不一致になる(Red)ことを確認した
    上で、`++x`に戻してGreen化した。
  - データ競合の実演デモ(`RunDataRaceWarningDemo`)は09番の`UnsafeCounter`と
    同様、非決定的な挙動そのものが学習ポイントであるため、自動テストの対象には
    せず`main.cpp`でのみ実演している。
- `ParallelAlgorithmsTests.exe`実行でテスト9件全てパスすることを複数回
  (連続5回実行しても常にパスすること)確認。
- `ParallelAlgorithms.exe`実行で、200万件のデータに対し`seq`/`par`/
  `par_unseq`全てで同じ結果になり、`par`/`par_unseq`が`seq`より高速になる
  傾向を確認した(実行環境依存のため必ず高速になるとは限らない旨も明記)。
  データ競合の実演では期待値(1,000,000)より少ない値(実測84,747)になり、
  lost updateを確認した。終了コード0で終わることも確認した。
  - 実装当初、`SumOfSquares`のデモで500万件のデータを使っていたところ、
    二乗の総和が`long long`の範囲(約9.2×10^18)を超えてオーバーフローする
    (符号付き整数オーバーフローは未定義動作)不具合に気づき、200万件に
    縮小して修正した。
