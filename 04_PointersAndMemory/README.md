# 04. ポインタ・参照・メモリ管理

## 目的
生ポインタ・参照の違いを理解した上で、スマートポインタとRAIIによる
安全なリソース管理の考え方を身につける。

## 学習ポイント
- 生ポインタ、参照、`nullptr`、ダングリングポインタ
- `std::unique_ptr` / `std::shared_ptr` / `std::weak_ptr`
- RAII（コンストラクタ/デストラクタでのリソース確保・解放）
- スタックとヒープ、メモリリークの典型例と対策

## 推奨ライブラリ/ツール
- 標準ライブラリ（`<memory>`）
- (任意) AddressSanitizer や Valgrind等でのリーク検出

## 成果物イメージ
`unique_ptr`/`shared_ptr` を使ったシンプルな連結リストまたはツリー構造の実装。
生ポインタ版と比較して、メモリ管理コードの違いを確認する。

## ビルド方法

01〜03と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 04_PointersAndMemory ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/PointersAndMemory
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `src/main.cpp` の `DemoRawPointerAndReference` | 生ポインタ・参照・`nullptr`・ダングリングポインタの解説 | 生ポインタ、参照、`nullptr`、ダングリングポインタ |
| `include/linked_list.h`/`.cpp` の `RawIntList` | 生ポインタ(`Node*`)による連結リスト。デストラクタで手動delete | スタックとヒープ、メモリリークの典型例 |
| `include/linked_list.h`/`.cpp` の `SmartIntList` | `std::unique_ptr`の連鎖による連結リスト。自動解放 | `std::unique_ptr` |
| `include/tree_node.h`/`.cpp` | 子は`shared_ptr`、親は`weak_ptr`で持つ木構造 | `std::shared_ptr`/`std::weak_ptr`、循環参照の回避 |
| `include/scoped_resource.h`/`.cpp` | コンストラクタ/デストラクタでログを出すRAIIクラス | RAII（例外発生時も解放が保証されることを実演） |

`AddressSanitizer`等でのリーク検出は学習ポイント上は任意のため、今回は導入していない。
`RawIntList`はデストラクタで正しく`delete`しているためリークしないが、
仮にデストラクタの実装を忘れた場合にリークすることを`src/linked_list.cpp`のコメントで説明している。
