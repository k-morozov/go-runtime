//
// Created by konstantin on 06.06.24.
//

#pragma once

#include <context/buffer/buffer.h>
#include <coro/stackfull_coro.h>
#include <executor/executor.h>
#include <executor/task/task_base.h>

namespace fibers {

class IAwaiter;

class AwaiterFiber final : public NExecutors::TaskBase {
public:
    AwaiterFiber(NExecutors::IExecutor* executor, coro::Routine routine,
                 ctx::Buffer&& buffer);

    void Schedule();

    void Suspend(IAwaiter* waiter);
    void Switch();

    void Run() noexcept override;

    static AwaiterFiber* Self();

    NExecutors::IExecutor* GetScheduler();

private:
    NExecutors::IExecutor* executor_;
    coro::StackfullCoroutine fiber_coro_;
    IAwaiter* awaiter_{nullptr};
};

}  // namespace fibers
