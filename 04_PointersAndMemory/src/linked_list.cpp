#include "linked_list.h"

#include <sstream>

// --- RawIntList ---

RawIntList::~RawIntList() {
    // 生ポインタなので、先頭からたどりながら1つずつ手動でdeleteする。
    Node* current = head_;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
}

void RawIntList::PushFront(int value) {
    Node* node = new Node{value, head_};
    head_ = node;
}

std::string RawIntList::ToString() const {
    std::ostringstream oss;
    oss << "[";
    for (const Node* current = head_; current != nullptr; current = current->next) {
        oss << current->value;
        if (current->next != nullptr) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

std::size_t RawIntList::Size() const {
    std::size_t count = 0;
    for (const Node* current = head_; current != nullptr; current = current->next) {
        ++count;
    }
    return count;
}

// --- SmartIntList ---

void SmartIntList::PushFront(int value) {
    // 新しいノードのnextに、これまでの先頭(head_)の所有権をstd::moveで譲渡する。
    // head_自体はムーブ後に空(nullptr)になるので、最後に新しいノードの所有権をhead_へ移す。
    auto node = std::make_unique<Node>(Node{value, std::move(head_)});
    head_ = std::move(node);
}

std::string SmartIntList::ToString() const {
    std::ostringstream oss;
    oss << "[";
    for (const Node* current = head_.get(); current != nullptr; current = current->next.get()) {
        oss << current->value;
        if (current->next != nullptr) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

std::size_t SmartIntList::Size() const {
    std::size_t count = 0;
    for (const Node* current = head_.get(); current != nullptr; current = current->next.get()) {
        ++count;
    }
    return count;
}
