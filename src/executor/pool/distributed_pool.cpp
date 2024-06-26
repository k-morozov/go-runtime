//
// Created by konstantin on 24.06.24.
//

#include "distributed_pool.h"

#include <executor/pool/internal/worker.h>

namespace NExecutors {

DistributedPool::DistributedPool(const size_t count_)
    : IExecutor(), count_workers(count_) {
    for (size_t i{}; i < count_workers; i++) {
        workers.emplace_back(this);
    }
}

DistributedPool::~DistributedPool() {
    shutdown_.store(true);
    WaitShutdown();
}

void DistributedPool::Start() {
    for (auto& w : workers) {
        w.Start();
    }
}

void DistributedPool::Submit(NExecutors::TaskBase* task) {
    if (shutdown_.load()) return;

    count_tasks_.fetch_add(1);
    const size_t worker_for_current_task = current_worker.fetch_add(1);
    workers[worker_for_current_task % count_workers].Push(task);
//    std::lock_guard lock(mutex);
//    global_tasks.Push(task);
}

IExecutor* DistributedPool::Current() { return internal::Worker::Current(); }

void DistributedPool::StartShutdown() {
    shutdown_.store(true);
}

void DistributedPool::WaitShutdown() {
    for (auto& w : workers) {
        w.Join();
    }
}

void DistributedPool::WaitIdle() {}

}  // namespace NExecutors