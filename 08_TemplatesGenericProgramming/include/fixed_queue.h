#ifndef FIXED_QUEUE_H
#define FIXED_QUEUE_H

#include <array>
#include <cstddef>
#include <stdexcept>

// 任意の型Tを、コンパイル時に決まる固定容量Capacityまで格納できるキュー(FIFO)。
// 内部はリングバッファ(循環バッファ)で実装しており、head/tailを折り返しながら使う。
template <typename T, std::size_t Capacity>
class FixedQueue {
public:
    void Enqueue(const T& value) {
        if (Full()) {
            throw std::out_of_range("FixedQueue::Enqueue: capacity exceeded");
        }
        data_[tail_] = value;
        tail_ = (tail_ + 1) % Capacity;
        ++size_;
    }

    T Dequeue() {
        if (Empty()) {
            throw std::out_of_range("FixedQueue::Dequeue: queue is empty");
        }
        const T value = data_[head_];
        head_ = (head_ + 1) % Capacity;
        --size_;
        return value;
    }

    const T& Front() const {
        if (Empty()) {
            throw std::out_of_range("FixedQueue::Front: queue is empty");
        }
        return data_[head_];
    }

    bool Empty() const { return size_ == 0; }
    bool Full() const { return size_ == Capacity; }
    std::size_t Size() const { return size_; }
    static constexpr std::size_t capacity() { return Capacity; }

private:
    std::array<T, Capacity> data_{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
};

#endif  // FIXED_QUEUE_H
