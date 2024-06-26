//
// Created by konstantin on 24.06.24.
//

#include "worker.h"

#include "executor.h"

namespace NExecutors::internal {

namespace {
thread_local IExecutor* CurrentPool;
}

Worker::Worker(IExecutor* ex) : ex(ex) {}

Worker::~Worker() {
    Join();
    assert(0 == count_local_tasks.load());
}

void Worker::Start() {
    thread.emplace(std::thread([this]() {
        while (true) {
            CurrentPool = ex;
            TaskBase* task{};
            {
                std::lock_guard lock(mutex);
                task = local_tasks.Pop();
            }

            if (task) {
                task->Run();
                std::lock_guard lock_task(mutex);
                count_local_tasks.fetch_sub(1);
                empty_tasks_.notify_one();
            } else {
                if (shutdown_worker.load()) {
                    break;
                }
            }
        }
    }));
}

void Worker::Join() {
    shutdown_worker.store(true);
    WaitIdle();
    if (thread && thread->joinable()) {
        thread->join();
    }
}

void Worker::Push(TaskBase* task) {
    if (shutdown_worker.load()) return;

    count_local_tasks.fetch_add(1);
    std::lock_guard lock(mutex);
    local_tasks.Push(task);
}

IExecutor* Worker::Current() { return CurrentPool; }

void Worker::WaitIdle() {
    std::unique_lock lock(mutex);
    while (0 != count_local_tasks.load()) {
        empty_tasks_.wait(lock);
    }
}

}  // namespace NExecutors::internal