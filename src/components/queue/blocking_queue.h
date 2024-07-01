//
// Created by konstantin on 10.05.24.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace NComponents {

// UnboundedBlockingQueue
template <class T>
class UnboundedBlockingQueue final {
public:
    bool Put(T value) {
        if (close_.load()) {
            return false;
        }

        std::lock_guard g(m_);
        data_.push(std::move(value));

        empty_data_.notify_one();
        return true;
    }

    std::optional<T> Take() {
        std::optional<T> result;

        std::unique_lock lock(m_);
        while (data_.empty() && !close_.load()) {
            empty_data_.wait(lock);
        }

        if (!data_.empty()) {
            result.emplace(std::move(data_.front()));
            data_.pop();
        }
        lock.unlock();

        empty_data_.notify_one();

        return result;
    }

    void Close() {
        std::unique_lock lock(m_);
        close_.store(true);
        empty_data_.notify_all();
    };

private:
    std::queue<T> data_;
    std::mutex m_;
    std::condition_variable empty_data_;
    std::atomic<bool> close_{false};
};

}  // namespace NComponents