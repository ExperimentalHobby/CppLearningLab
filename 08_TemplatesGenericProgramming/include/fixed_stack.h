#ifndef FIXED_STACK_H
#define FIXED_STACK_H

#include <array>
#include <cstddef>
#include <stdexcept>

// 任意の型Tを、コンパイル時に決まる固定容量Capacityまで格納できるスタック。
//
// クラステンプレートの定義は原則としてヘッダーに書く必要がある。
// 03課題(関数・スコープ)ではヘッダー(宣言)とソース(実装)を分割したが、
// テンプレートはインスタンス化(例: FixedStack<int, 8>)されて初めて実体化するコードであり、
// コンパイラは使用箇所ごとに定義全体を必要とする。実装を.cppに分離すると、
// その.cppをコンパイルする時点ではどの型でインスタンス化されるか分からずリンクエラーになる。
template <typename T, std::size_t Capacity>
class FixedStack {
public:
    void Push(const T& value) {
        if (Full()) {
            throw std::out_of_range("FixedStack::Push: capacity exceeded");
        }
        data_[size_] = value;
        ++size_;
    }

    T Pop() {
        if (Empty()) {
            throw std::out_of_range("FixedStack::Pop: stack is empty");
        }
        --size_;
        return data_[size_];
    }

    const T& Top() const {
        if (Empty()) {
            throw std::out_of_range("FixedStack::Top: stack is empty");
        }
        return data_[size_ - 1];
    }

    bool Empty() const { return size_ == 0; }
    bool Full() const { return size_ == Capacity; }
    std::size_t Size() const { return size_; }
    static constexpr std::size_t capacity() { return Capacity; }

private:
    std::array<T, Capacity> data_{};
    std::size_t size_ = 0;
};

#endif  // FIXED_STACK_H
