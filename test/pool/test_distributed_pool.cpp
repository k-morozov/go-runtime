//
// Created by konstantin on 25.06.24.
//

#include "gtest/gtest.h"

#include <thread>

#include <executor/pool/distributed_pool.h>
#include <fiber/sync/wait_group.h>
#include <go/go.h>
#include <fiber/sync/async_mutex.h>

#include "common/clock.h"

using namespace std::chrono_literals;

TEST(TestDistributedPool, JustWorks) {
    NExecutors::DistributedPool pool{4};

    pool.Start();

    fibers::Go(pool, [] {
        fibers::WaitGroup wg;
        wg.Add(1);

        fibers::Go([&wg] {
            std::cout << "Hello from thread pool!" << std::endl;
            wg.Done();
        });
        wg.Wait();
    });
}

TEST(TestDistributedPool, Wait) {
    NExecutors::DistributedPool pool{4};

    pool.Start();

    fibers::Go(pool, [] {
        fibers::WaitGroup wg;
        wg.Add(1);

        fibers::Go([&] {
            std::this_thread::sleep_for(1s);
            wg.Done();
        });

        wg.Wait();
    });
}

TEST(TestDistributedPool, MultiWait) {
    NExecutors::DistributedPool pool{4};

    pool.Start();

    fibers::Go(pool, [] {
        for (size_t i = 0; i < 3; ++i) {
            fibers::WaitGroup wg;
            wg.Add(1);

            fibers::Go([&] {
                std::this_thread::sleep_for(1s);
                wg.Done();
            });

            wg.Wait();
        }
    });
}

TEST(TestDistributedPool, ManyTasks) {
    NExecutors::DistributedPool pool{4};

    pool.Start();

    fibers::Go(pool, [] {
        static const size_t kTasks = 17;

        fibers::WaitGroup wg;
        wg.Add(kTasks);

        for (size_t i = 0; i < kTasks; ++i) {
            fibers::Go([&] {
                wg.Done();
            });
        }

        wg.Wait();
    });
}

TEST(TestDistributedPool, DoNotBurnCPU) {
    NExecutors::DistributedPool pool{4};
    
    pool.Start();
    
    fibers::Go(pool, [] {
        fibers::WaitGroup wg;
        wg.Add(4);
        
        common::ProcessCPUTimer cpu_timer;

        // Warmup
        for (size_t i = 0; i < 4; ++i) {
            fibers::Go([&] {
                std::this_thread::sleep_for(100ms);
                wg.Done();
            });
        }

        wg.Wait();
        
        ASSERT_TRUE(cpu_timer.Spent() < 100ms);
    });
}

TEST(TestDistributedPool, Current) {
    NExecutors::DistributedPool pool{4};
    
    pool.Start();
    
    fibers::Go(pool, [&] {
        fibers::WaitGroup wg;
        wg.Add(1);

        fibers::Go([&] {
            ASSERT_EQ(NExecutors::DistributedPool::Current(), &pool);
            wg.Done();
        });

        wg.Wait();
    });
}

TEST(TestDistributedPool, SubmitAfterWait) {
    NExecutors::DistributedPool pool{4};
    
    pool.Start();
    
    fibers::Go(pool, [] {
        fibers::WaitGroup wg;
        wg.Add(1);

        fibers::Go([&] {
            std::this_thread::sleep_for(500ms);

            fibers::Go(*NExecutors::DistributedPool::Current(), [&] {
                std::this_thread::sleep_for(500ms);
                wg.Done();
            });
        });

        wg.Wait();
    });
}

TEST(TestDistributedPool, Racy) {
    NExecutors::DistributedPool pool{4};

    pool.Start();

    fibers::Go(pool, [] {
        static const size_t kTasks = 100500;

        std::atomic<size_t> shared_counter{0};

        fibers::WaitGroup wg;
        wg.Add(kTasks);

        for (size_t i = 0; i < kTasks; ++i) {
            fibers::Go([&] {
                const size_t old = shared_counter.load();
                shared_counter.store(old + 1);

                wg.Done();
            });
        }

        wg.Wait();

        std::cout << "Racy counter value: " << shared_counter << std::endl;

        ASSERT_LE(shared_counter.load(), kTasks);
    });
}

TEST(TestDistributedPool, ForBench) {
    NExecutors::DistributedPool pool{4};

    static const size_t kTasks = 1'000;

    pool.Start();

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
        ASSERT_EQ(counter, kTasks);
    });
}
