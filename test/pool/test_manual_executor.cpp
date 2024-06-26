//
// Created by konstantin on 05.06.24.
//

#include "gtest/gtest.h"

#include <memory>

#include <executor/manual/intrusive_manual_executor.h>
#include <executor/submit.h>

namespace {

class Looper {
public:
    explicit Looper(NExecutors::IExecutor& e, size_t iters)
        : executor_(e), iters_left_(iters) {}

    void Start() { Submit(); }

    void Iter() {
        --iters_left_;
        if (iters_left_ > 0) {
            Submit();
        }
    }

private:
    void Submit() {
        NExecutors::Submit(executor_, [this] { Iter(); });
    }

private:
    NExecutors::IExecutor& executor_;
    size_t iters_left_;
};

class LooperManyTasks {
public:
    explicit LooperManyTasks(NExecutors::IExecutor& e, size_t count)
        : executor_(e), count_(count) {}

    void Start() {
        for (size_t i = 0; i < count_; i++) {
            Submit();
        }
    }

    void Iter() { counter_++; }

    size_t GetCount() const { return counter_; }

private:
    void Submit() {
        NExecutors::Submit(executor_, [this] { Iter(); });
    }

private:
    NExecutors::IExecutor& executor_;
    const size_t count_;
    size_t counter_{0};
};

}  // namespace

TEST(TestManualExecutor, JustWorks) {
    NExecutors::IntrusiveManualExecutor manual;

    size_t step = 0;

    ASSERT_TRUE(manual.IsEmpty());
    ASSERT_FALSE(manual.NonEmpty());

    ASSERT_FALSE(manual.RunNext());
    ASSERT_EQ(manual.RunAtMost(99), 0u);

    NExecutors::Submit(manual, [&] { step = 1; });

    ASSERT_FALSE(manual.IsEmpty());
    ASSERT_TRUE(manual.NonEmpty());
    ASSERT_EQ(manual.TaskCount(), 1u);

    ASSERT_EQ(step, 0u);

    NExecutors::Submit(manual, [&] { step = 2; });

    ASSERT_EQ(manual.TaskCount(), 2u);

    ASSERT_EQ(step, 0u);

    ASSERT_TRUE(manual.RunNext());

    ASSERT_EQ(step, 1u);

    ASSERT_FALSE(manual.IsEmpty());
    ASSERT_TRUE(manual.NonEmpty());
    ASSERT_EQ(manual.TaskCount(), 1u);

    NExecutors::Submit(manual, [&] { step = 3u; });

    ASSERT_EQ(manual.TaskCount(), 2u);

    ASSERT_EQ(manual.RunAtMost(99), 2u);
    ASSERT_EQ(step, 3u);

    ASSERT_TRUE(manual.IsEmpty());
    ASSERT_FALSE(manual.NonEmpty());
    ASSERT_FALSE(manual.RunNext());
}

TEST(TestManualExecutor, RunAtMost1) {
    NExecutors::IntrusiveManualExecutor manual;

    const size_t all_tasks = 256;

    Looper looper{manual, all_tasks};
    looper.Start();

    size_t tasks = 0;
    const size_t step = 7;
    do {
        tasks += manual.RunAtMost(step);
    } while (manual.NonEmpty());

    ASSERT_EQ(tasks, all_tasks);
}

TEST(TestManualExecutor, RunAtMost2) {
    NExecutors::IntrusiveManualExecutor manual;

    const size_t all_tasks = 256;
    size_t remaining_tasks = all_tasks;

    LooperManyTasks looper{manual, all_tasks};
    looper.Start();

    size_t tasks = 0;
    const size_t step = 7;
    do {
        tasks += manual.RunAtMost(step);
        remaining_tasks = (remaining_tasks > step ? remaining_tasks - step : 0);
        ASSERT_EQ(remaining_tasks, manual.TaskCount());
    } while (manual.NonEmpty());

    ASSERT_EQ(tasks, all_tasks);
    ASSERT_TRUE(0u != looper.GetCount());
    ASSERT_EQ(looper.GetCount(), all_tasks);
}

TEST(TestManualExecutor, Drain) {
    NExecutors::IntrusiveManualExecutor manual;

    Looper looper{manual, 117};
    looper.Start();

    ASSERT_EQ(manual.Drain(), 117u);
}