//
// Created by konstantin on 08.06.24.
//

#pragma once

#include <mutex>

#include <components/sync/spinLock.h>
#include <components/intrusive/list.h>
#include <fiber/awaiter/awaiter.h>

namespace fibers {

template <class M>
class AsyncMutexWaiter : public IAwaiter,
                         public NComponents::Node<AsyncMutexWaiter<M>> {
public:
    using Guard = std::unique_lock<typename M::Spinlock>;

    AsyncMutexWaiter(M* mutex, Guard guard)
        : mutex(mutex), guard(std::move(guard)){};

    void AwaitSuspend(StoppedFiber handle) override {
        assert(handle.IsValid());

        stopped_handle = handle;
        mutex->Park(this);
        guard.release()->unlock();
    }

    void Schedule() { stopped_handle.Schedule(); }

private:
    M* mutex;
    StoppedFiber stopped_handle;
    Guard guard;
};

}  // namespace fibers
