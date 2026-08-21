#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <memory>
#include <string>

// 生ポインタで実装した単方向連結リスト。
// ノードの所有権を誰も明示的に表現しないため、デストラクタで手動にたどってdeleteする
// 必要がある。ここでdeleteを書き忘れる、あるいは例外発生時にdeleteに到達しないと
// メモリリークになる。
class RawIntList {
public:
    RawIntList() = default;
    ~RawIntList();

    // コピーすると同じノードを二重に所有してしまう(二重解放の原因)ため、
    // 学習用にコピーを禁止しておく。
    RawIntList(const RawIntList&) = delete;
    RawIntList& operator=(const RawIntList&) = delete;

    void PushFront(int value);
    std::string ToString() const;
    std::size_t Size() const;

private:
    struct Node {
        int value;
        Node* next;
    };

    Node* head_ = nullptr;
};

// std::unique_ptrの連鎖で実装した単方向連結リスト。
// 各ノードが次のノードの所有権をunique_ptrで表現するため、リスト(および先頭ノード)が
// 破棄されると連鎖的に自動でノードが解放される。デストラクタを自分で書く必要がない。
class SmartIntList {
public:
    void PushFront(int value);
    std::string ToString() const;
    std::size_t Size() const;

private:
    struct Node {
        int value;
        std::unique_ptr<Node> next;
    };

    std::unique_ptr<Node> head_;
};

#endif  // LINKED_LIST_H
