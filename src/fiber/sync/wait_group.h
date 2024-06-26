//
// Created by konstantin on 09.06.24.
//

#pragma once

#include <mutex>

#include <fiber/awaiter/wait_group_awaiter.h>
#include <components/intrusive/list.h>
#include <components/sync/spinLock.h>

namespace fibers {

class WaitGroup {
    using Spinlock = NSync::SpinLock;
    using Waiter = WaitGroupWaiter<WaitGroup>;

    friend Waiter;

public:
    ~WaitGroup() {
        assert(wg_waiters_.IsEmpty());
    }

    void Add(size_t);
    void Done();
    void Wait();

private:
    size_t counter_{0};
    Spinlock spinlock_;

    NComponents::IntrusiveList<Waiter> wg_waiters_;

    void Park(Waiter* waiter);
};

}  // namespace fibers
