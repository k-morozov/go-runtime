//
// Created by konstantin on 25.06.24.
//

#include <thread>
#include <vector>

#include <benchmark/benchmark.h>
#include <executor/pool/distributed_pool.h>
#include <executor/pool/intrusive_pool.h>
#include <fiber/sync/async_mutex.h>
#include <fiber/sync/wait_group.h>
#include <go/go.h>

const size_t CountThreads = std::thread::hardware_concurrency();
static constexpr size_t kTasks = 10000;
static constexpr size_t CountIteration = 5;

void bench_logic(NExecutors::IExecutor& pool) {
    size_t counter{};

    fibers::Go(pool, [&counter] {
        fibers::AsyncMutex mutex;
        fibers::WaitGroup wg;

        wg.Add(kTasks);

        for (size_t i{}; i < kTasks; i++) {
            fibers::Go([&] {
                {
                    std::lock_guard lock(mutex);
                    counter++;
                }
                wg.Done();
            });
        }

        wg.Wait();
        assert(counter == kTasks);
    });
}

static void IntrusiveThreadPool(benchmark::State& state) {
    for (auto _ : state) {
        NExecutors::IntrusiveThreadPool pool{CountThreads};
        pool.Start();
        bench_logic(pool);
    }
}
BENCHMARK(IntrusiveThreadPool)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(CountIteration);

static void DistributedPool(benchmark::State& state) {
    for (auto _ : state) {
        NExecutors::DistributedPool pool{CountThreads};
        pool.Start();
        bench_logic(pool);
    }
}
BENCHMARK(DistributedPool)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(CountIteration);