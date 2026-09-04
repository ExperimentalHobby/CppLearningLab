# 25. 簡易ORMラッパー設計

## 目的
テーブルとC++クラス（オブジェクト）の対応付けを自作し、生SQLを直接書かずに
データ操作ができる簡易ORM風の抽象化レイヤーを設計する。

## 学習ポイント
- テーブル⇔オブジェクトのマッピング設計
- テンプレート/ジェネリックプログラミング（08番）を活かした汎用的なRepositoryクラス設計
- 生SQLとORM抽象化のトレードオフ（デバッグのしやすさ、パフォーマンス等）

## 推奨ライブラリ/ツール
- 21/22で使用したDBライブラリの上に自作のラッパーを構築

本課題では21番と同じ**SQLite3**（[third_party/sqlite3](../third_party/sqlite3/NOTICE.md)の
amalgamationを共有利用）の上に自作のRepositoryラッパーを構築する。

## 成果物イメージ
`Repository<T>`のようなテンプレートクラスを設計し、`find`, `save`, `remove`
といった汎用メソッドで複数のエンティティ（例: User, Product）を扱えるようにする。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 25_ORMWrapperDesign ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/ORMWrapperDesign
```

## 設計

エンティティ固有の知識(テーブル名・SQL・列とメンバのバインド方法)を全て
「Mapperポリシークラス」側に閉じ込め、`Repository<T, Mapper>`本体はSQL文字列を
一切知らない汎用的な`FindAll`/`FindById`/`Save`/`Remove`だけを提供する設計にした。

```cpp
template <typename T, typename Mapper>
class Repository {
   public:
    explicit Repository(sqlite3* db);
    void EnsureSchema();
    std::vector<T> FindAll() const;
    bool FindById(long long id, T& out) const;
    long long Save(T& entity) const;  // id==0ならINSERT、それ以外はUPDATE
    bool Remove(long long id) const;
};
```

`Mapper`(`UserMapper`/`ProductMapper`)は以下の静的メンバ関数を持つだけの
ただの構造体で、継承もインターフェースも使わない。`Repository`側は
テンプレート引数として渡された型がこれらの関数を持つことを前提にコンパイル時に
解決する（C++17なのでconcepts言語機能は使わず、コンパイル時ダックタイピングで
表現している）。

```cpp
struct UserMapper {
    static std::string TableName();
    static std::string CreateTableSql();
    static std::string SelectColumnsSql();   // idを除く列名, 例: "name, email"
    static int ColumnCount();
    static void BindInsertParams(sqlite3_stmt*, const User&);
    static void BindUpdateParams(sqlite3_stmt*, const User&);
    static User FromStatement(sqlite3_stmt*);
    static long long GetId(const User&);
    static void SetId(User&, long long);
};
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `UserMapper`/`ProductMapper`(`user.h`/`product.h`) | テーブル名・SQL・列とメンバのマッピング定義 | テーブル⇔オブジェクトのマッピング設計 |
| `Repository<T, Mapper>`(`repository.h`) | Mapperを型パラメータに取り、SQLを知らない汎用CRUDを提供 | テンプレート/ジェネリックプログラミング(08番)を活かした汎用的なRepositoryクラス設計 |
| `src/main.cpp` | `Repository<User, UserMapper>`と`Repository<Product, ProductMapper>`を同じDBで使い分ける | 生SQLとORM抽象化のトレードオフの実感(呼び出し側からSQLが完全に消える一方、`Mapper`側の実装コストは残る) |

## 動作確認

ビルド・実行し、以下を確認済み。

- `Repository<User, UserMapper>`: 追加(id自動採番)→一覧→更新(emailの変更が
  反映される)→削除(該当ユーザーが一覧から消える)
- `Repository<Product, ProductMapper>`: 追加→一覧→`FindById`による単体検索
- 同一の`Repository<T, Mapper>`テンプレートが、列数も列の型(`name`+`email`の
  文字列2つ vs `name`+`price`の文字列と整数)も異なる2種類のエンティティを
  問題なく扱えること
