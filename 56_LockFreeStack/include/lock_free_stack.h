// std::atomic<Node*>+compare_exchange_weakによるロックフリースタック
// (Treiber stack)。08番のFixedStack/09番のBlockingQueue<T>と同様、
// テンプレートのためヘッダーオンリーで実装する。
//
// メモリ回収の簡略化: Pop()で取り外したノードをその場でdeleteすると、
// 他スレッドがまだそのノードを参照中(CASリトライの途中でnext等を読んで
// いる)にuse-after-freeになりうる。本実装は教育目的の簡略化として、
// 取り外したノードを別のアトミックな連結リスト(retiredHead_)に退避する
// だけに留め、実際の解放はデストラクタ(全スレッド合流後が前提)でまとめて
// 行う。ハザードポインタやエポックベース回収等の本格的な安全回収は
// スコープ外。
//
// ABA問題: compare_exchangeは「値(ポインタのアドレス)」だけを比較するため、
// あるポインタ値が一度別の用途に使われた後、たまたま同じアドレスに戻って
// きた場合に誤って成功したと判定してしまう可能性がある。本実装は上記の
// 遅延回収によりノードのメモリを実際には再利用(alloc使い回し)しないため
// このスコープでは実害が出にくいが、タグ付きポインタ等による汎用的な解決は
// 行っていない。
#pragma once

#include <atomic>
#include <optional>
#include <utility>

namespace concurrency {

template <typename T>
class LockFreeStack {
   public:
    LockFreeStack() = default;
    ~LockFreeStack() {
        FreeChain(head_.load(std::memory_order_relaxed));
        FreeChain(retiredHead_.load(std::memory_order_relaxed));
    }

    LockFreeStack(const LockFreeStack&) = delete;
    LockFreeStack& operator=(const LockFreeStack&) = delete;

    void Push(T value) {
        Node* newNode = new Node{std::move(value), head_.load(std::memory_order_relaxed)};
        while (!head_.compare_exchange_weak(newNode->next, newNode, std::memory_order_release,
                                             std::memory_order_relaxed)) {
        }
    }

    std::optional<T> Pop() {
        Node* oldHead = head_.load(std::memory_order_acquire);
        // compare_exchange_weakが失敗した場合、oldHeadは参照渡しで最新の
        // head_の値に更新される。失敗の原因が「別スレッドが割り込んで
        // head_を変えた」ことであり、その最新値がnullptrでない限り
        // (=スタックがまだ空でない限り)、その値でリトライし続ける。
        while (oldHead && !head_.compare_exchange_weak(oldHead, oldHead->next, std::memory_order_acquire,
                                                         std::memory_order_relaxed)) {
        }
        if (!oldHead) {
            return std::nullopt;
        }
        T value = std::move(oldHead->value);
        Retire(oldHead);
        return value;
    }

   private:
    struct Node {
        T value;
        Node* next;
    };

    void Retire(Node* node) {
        Node* oldRetired = retiredHead_.load(std::memory_order_relaxed);
        do {
            node->next = oldRetired;
        } while (!retiredHead_.compare_exchange_weak(oldRetired, node, std::memory_order_release,
                                                       std::memory_order_relaxed));
    }

    static void FreeChain(Node* node) {
        while (node) {
            Node* next = node->next;
            delete node;
            node = next;
        }
    }

    std::atomic<Node*> head_{nullptr};
    std::atomic<Node*> retiredHead_{nullptr};
};

}  // namespace concurrency
