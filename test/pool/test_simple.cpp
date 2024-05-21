//
// Created by konstantin on 09.05.24.
//

#include "gtest/gtest.h"

#include <memory>
#include <thread>

#include <pool/executor/executor.h>
#include <pool/task/task.h>

using namespace std::chrono_literals;

class TestSimple : public ::testing::Test {
public:
};

struct TestTask : pool::Task {
    void Run() override {
        completed = true;
        ++counter;
    }

    ~TestTask() override {
        EXPECT_FALSE(counter >= 2) << "seems like task body was run multiple times";
    }

    bool completed = false;
    int counter = 0;
};

template <size_t msec>
struct SlowTask : pool::Task {
    void Run() override {
        std::this_thread::sleep_for(msec * 1ms);
        completed = true;
    }

    bool completed = false;
};

struct FailedTask : pool::Task {
    void Run() override { throw std::logic_error{"Failed"}; }
};

TEST_F(TestSimple, Destructor) { auto _ = pool::MakeThreadPool(4); }

TEST_F(TestSimple, StartShutdown) {
    auto pool = pool::MakeThreadPool(4);
    pool->StartShutdown();
}

TEST_F(TestSimple, StartTwiceAndWait) {
    auto pool = pool::MakeThreadPool(4);
    pool->StartShutdown();
    pool->StartShutdown();
    pool->WaitShutdown();
}

TEST_F(TestSimple, RunSingleTask) {
    auto pool = pool::MakeThreadPool(4);

    auto task = std::make_shared<TestTask>();

    pool->Submit(task);
    task->Wait();

    ASSERT_TRUE(task->completed);
    ASSERT_TRUE(task->IsFinished());
    ASSERT_TRUE(task->IsCompleted());
    ASSERT_FALSE(task->IsCanceled());
    ASSERT_FALSE(task->IsFailed());
}

TEST_F(TestSimple, RunSingleFailingTask) {
    auto pool = pool::MakeThreadPool(4);

    auto task = std::make_shared<FailedTask>();

    pool->Submit(task);
    task->Wait();

    ASSERT_FALSE(task->IsCompleted());
    ASSERT_FALSE(task->IsCanceled());
    ASSERT_TRUE(task->IsFailed());

    auto error = task->GetError();
    EXPECT_THROW(std::rethrow_exception(error), std::logic_error);
}

TEST_F(TestSimple, CancelSingleTask) {
    auto pool = pool::MakeThreadPool(4);

    auto task = std::make_shared<TestTask>();
    task->Cancel();
    task->Wait();

    ASSERT_FALSE(task->IsCompleted());
    ASSERT_TRUE(task->IsCanceled());
    ASSERT_FALSE(task->IsFailed());

    pool->Submit(task);
    task->Wait();

    ASSERT_FALSE(task->IsCompleted());
    ASSERT_TRUE(task->IsCanceled());
    ASSERT_FALSE(task->IsFailed());
}