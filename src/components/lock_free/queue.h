//
// Created by konstantin on 01.07.24.
//

#pragma once

#include <atomic>
#include <optional>

namespace NComponents {

/**
 * Simple, Fast, and Practical Non-Blocking and Blocking
 * Concurrent Queue Algorithms
 */
template <class T>
class LockFreeQueue final {
    struct NodePointer {
        NodePointer* ptr{};
        size_t counter{};
    };

    using AtomicNodePointer = std::atomic<NodePointer*>;

    struct Node {
        T value;
        std::atomic<NodePointer> next;
    };

private:
    AtomicNodePointer head;
    AtomicNodePointer tail;

public:
    LockFreeQueue() {
        Node* dummy = new Node();
        dummy->next.ptr = nullptr;
        head = tail = dummy;
    }
    ~LockFreeQueue() {

    }

    void Push(T item) {
        auto* new_node = new Node();
        new_node->value = std::move(item);
        new_node->next.ptr = nullptr;

        while(true) {
            Node* old_tail = tail.load();
            Node* next = old_tail->next.load();

            if (tail == tail.load()) {
                if (next->ptr == nullptr) {
                    if (tail->next.compare_exchange_strong(tail, {new_node, next + 1})) {
                        break;
                    }
                } else {

                }
            }
        }
    }

    std::optional<T> TryPop();
};

}  // namespace NComponents
