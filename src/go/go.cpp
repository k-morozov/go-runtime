//
// Created by konstantin on 29.05.24.
//

#include "go.h"

#include <context/buffer/buffer.h>
#include <fiber/fiber.h>

static const size_t kDefaultCoroBufferSize = 64 * 1024;

namespace fibers {

void Go(pool::ThreadPool& scheduler, coro::Routine routine) {
    auto buffer = ctx::Buffer::AllocBytes(kDefaultCoroBufferSize);
    auto* fiber = new Fiber(&scheduler, std::move(routine), std::move(buffer));
    fiber->Schedule();
}

void Go(coro::Routine routine) {
    Go(*Fiber::Self()->GetScheduler(), std::move(routine));
}

void Yield() { Fiber::Self()->Yield(); }

}  // namespace fibers