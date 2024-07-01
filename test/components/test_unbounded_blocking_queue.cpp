//
// Created by konstantin on 10.05.24.
//

#include "gtest/gtest.h"

#include <chrono>
#include <thread>

#include <common/clock.h>

#include <components/queue/blocking_queue.h>

using namespace std::chrono_literals;

template <typename T>
using Queue = NComponents::UnboundedBlockingQueue<T>;

TEST(TestPoolSimple, JustWorks) {
    Queue<int> queue;

    ASSERT_TRUE(queue.Put(7));

    auto value = queue.Take();
    ASSERT_TRUE(value);
    ASSERT_EQ(*value, 7);

    queue.Close();
    ASSERT_FALSE(queue.Take());
}

TEST(TestPoolSimple, Fifo) {
    Queue<int> queue;
    queue.Put(1);
    queue.Put(2);
    queue.Put(3);

    ASSERT_EQ(*queue.Take(), 1);
    ASSERT_EQ(*queue.Take(), 2);
    ASSERT_EQ(*queue.Take(), 3);
}

TEST(TestPoolSimple, Close) {
    Queue<std::string> queue;

    queue.Put("Hello");
    queue.Put(",");
    queue.Put("World");

    queue.Close();

    ASSERT_FALSE(queue.Put("!"));

    ASSERT_EQ(*queue.Take(), "Hello");
    ASSERT_EQ(*queue.Take(), ",");
    ASSERT_EQ(*queue.Take(), "World");
    ASSERT_FALSE(queue.Take());
}

struct MoveOnly {
    MoveOnly() = default;

    MoveOnly(const MoveOnly& that) = delete;
    MoveOnly& operator=(const MoveOnly& that) = delete;

    MoveOnly(MoveOnly&& that) = default;
    MoveOnly& operator=(MoveOnly&& that) = default;
};

TEST(TestPoolSimple, MoveOnly) {
    Queue<MoveOnly> queue;

    queue.Put(MoveOnly{});
    ASSERT_TRUE(queue.Take().has_value());
}

TEST(TestPoolSimple, BlockingTake) {
    Queue<int> queue;

    std::thread producer([&]() {
        std::this_thread::sleep_for(1s);
        queue.Put(7);
    });

    common::ThreadCPUTimer thread_cpu_timer;

    auto value = queue.Take();

    auto spent = thread_cpu_timer.Spent();

    ASSERT_TRUE(value);
    ASSERT_EQ(*value, 7);
    ASSERT_LE(spent, 100ms);

    producer.join();
}

TEST(TestPoolSimple, BlockingTake2) {
    Queue<int> queue;

    std::thread producer([&]() {
        std::this_thread::sleep_for(1s);
        queue.Close();
    });

    common::ThreadCPUTimer thread_cpu_timer;

    auto value = queue.Take();

    auto spent = thread_cpu_timer.Spent();

    ASSERT_FALSE(value);
    ASSERT_LE(spent, 100ms);

    producer.join();
}

TEST(TestPoolSimple, UnblockConsumers) {
    Queue<int> queue;

    // Consumers

    std::thread consumer1([&]() { queue.Take(); });

    std::thread consumer2([&]() { queue.Take(); });

    // Producer
    std::this_thread::sleep_for(100ms);
    queue.Close();

    consumer1.join();
    consumer2.join();
}

TEST(TestPoolSimple, ProducerConsumer) {
    Queue<int> queue;

    common::ProcessCPUTimer process_cpu_timer;

    std::thread producer([&]() {
        // Producer
        for (int i = 0; i < 10; ++i) {
            queue.Put(i);
            std::this_thread::sleep_for(100ms);
        }
        queue.Close();
    });

    // Consumer

    for (int i = 0; i < 10; ++i) {
        auto value = queue.Take();
        ASSERT_TRUE(value);
        ASSERT_EQ(*value, i);
    }

    ASSERT_FALSE(queue.Take());

    producer.join();

    ASSERT_LE(process_cpu_timer.Spent(), 100ms);
}