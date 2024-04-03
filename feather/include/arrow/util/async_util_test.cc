// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "arrow/util/async_util.h"

#include <thread>

#include <gtest/gtest.h>

#include "arrow/result.h"
#include "arrow/testing/future_util.h"
#include "arrow/testing/gtest_util.h"

namespace arrow {
namespace util {

class GatingDestroyable : public AsyncDestroyable {
 public:
  GatingDestroyable(Future<> close_future, bool* destroyed)
      : close_future_(std::move(close_future)), destroyed_(destroyed) {}
  ~GatingDestroyable() override { *destroyed_ = true; }

 protected:
  Future<> DoDestroy() override { return close_future_; }

 private:
  Future<> close_future_;
  bool* destroyed_;
};

template <typename Factory>
void TestAsyncDestroyable(Factory factory) {
  Future<> gate = Future<>::Make();
  bool destroyed = false;
  bool on_closed = false;
  {
    auto obj = factory(gate, &destroyed);
    obj->on_closed().AddCallback([&](const Status& st) { on_closed = true; });
    ASSERT_FALSE(destroyed);
  }
  ASSERT_FALSE(destroyed);
  ASSERT_FALSE(on_closed);
  gate.MarkFinished();
  ASSERT_TRUE(destroyed);
  ASSERT_TRUE(on_closed);
}

TEST(AsyncDestroyable, MakeShared) {
  TestAsyncDestroyable([](Future<> gate, bool* destroyed) {
    return MakeSharedAsync<GatingDestroyable>(gate, destroyed);
  });
}

// The next four tests are corner cases but can sometimes occur when using these types
// in standard containers on certain versions of the compiler/cpplib.  Basically we
// want to make sure our deleter is ok with null pointers.
TEST(AsyncDestroyable, DefaultUnique) {
  std::unique_ptr<GatingDestroyable, DestroyingDeleter<GatingDestroyable>> default_ptr;
  default_ptr.reset();
}

TEST(AsyncDestroyable, NullUnique) {
  std::unique_ptr<GatingDestroyable, DestroyingDeleter<GatingDestroyable>> null_ptr(
      nullptr);
  null_ptr.reset();
}

TEST(AsyncDestroyable, NullShared) {
  std::shared_ptr<GatingDestroyable> null_ptr(nullptr,
                                              DestroyingDeleter<GatingDestroyable>());
  null_ptr.reset();
}

TEST(AsyncDestroyable, NullUniqueToShared) {
  std::unique_ptr<GatingDestroyable, DestroyingDeleter<GatingDestroyable>> null_ptr(
      nullptr);
  std::shared_ptr<GatingDestroyable> null_shared = std::move(null_ptr);
  null_shared.reset();
}

TEST(AsyncDestroyable, MakeUnique) {
  TestAsyncDestroyable([](Future<> gate, bool* destroyed) {
    return MakeUniqueAsync<GatingDestroyable>(gate, destroyed);
  });
}

template <typename T>
class TypedTestAsyncTaskGroup : public ::testing::Test {};

using AsyncTaskGroupTypes = ::testing::Types<AsyncTaskGroup, SerializedAsyncTaskGroup>;

TYPED_TEST_SUITE(TypedTestAsyncTaskGroup, AsyncTaskGroupTypes);

TYPED_TEST(TypedTestAsyncTaskGroup, Basic) {
  TypeParam task_group;
  Future<> fut1 = Future<>::Make();
  Future<> fut2 = Future<>::Make();
  ASSERT_OK(task_group.AddTask([fut1]() { return fut1; }));
  ASSERT_OK(task_group.AddTask([fut2]() { return fut2; }));
  Future<> all_done = task_group.End();
  AssertNotFinished(all_done);
  fut1.MarkFinished();
  AssertNotFinished(all_done);
  fut2.MarkFinished();
  ASSERT_FINISHES_OK(all_done);
}

TYPED_TEST(TypedTestAsyncTaskGroup, NoTasks) {
  TypeParam task_group;
  ASSERT_FINISHES_OK(task_group.End());
}

TYPED_TEST(TypedTestAsyncTaskGroup, OnFinishedDoesNotEnd) {
  TypeParam task_group;
  Future<> on_finished = task_group.OnFinished();
  AssertNotFinished(on_finished);
  ASSERT_FINISHES_OK(task_group.End());
  ASSERT_FINISHES_OK(on_finished);
}

TYPED_TEST(TypedTestAsyncTaskGroup, AddAfterDone) {
  TypeParam task_group;
  ASSERT_FINISHES_OK(task_group.End());
  ASSERT_RAISES(Cancelled, task_group.AddTask([] { return Future<>::Make(); }));
}

TYPED_TEST(TypedTestAsyncTaskGroup, AddAfterEndButBeforeFinish) {
  TypeParam task_group;
  Future<> task_one = Future<>::Make();
  ASSERT_OK(task_group.AddTask([task_one] { return task_one; }));
  Future<> finish_fut = task_group.End();
  AssertNotFinished(finish_fut);
  ASSERT_RAISES(Cancelled, task_group.AddTask([] { return Future<>::Make(); }));
  AssertNotFinished(finish_fut);
  task_one.MarkFinished();
  AssertFinished(finish_fut);
  ASSERT_FINISHES_OK(finish_fut);
}

TYPED_TEST(TypedTestAsyncTaskGroup, Error) {
  TypeParam task_group;
  Future<> failed_task = Future<>::MakeFinished(Status::Invalid("XYZ"));
  ASSERT_RAISES(Invalid, task_group.AddTask([failed_task] { return failed_task; }));
  ASSERT_FINISHES_AND_RAISES(Invalid, task_group.End());
}

TYPED_TEST(TypedTestAsyncTaskGroup, ErrorWhileNotEmpty) {
  TypeParam task_group;
  Future<> pending_task = Future<>::Make();
  Future<> will_fail_task = Future<>::Make();
  Future<> after_fail_task = Future<>::Make();
  ASSERT_OK(task_group.AddTask([pending_task] { return pending_task; }));
  ASSERT_OK(task_group.AddTask([will_fail_task] { return will_fail_task; }));
  ASSERT_OK(task_group.AddTask([after_fail_task] { return after_fail_task; }));
  Future<> end = task_group.End();
  AssertNotFinished(end);
  pending_task.MarkFinished();
  will_fail_task.MarkFinished(Status::Invalid("XYZ"));
  after_fail_task.MarkFinished();
  ASSERT_FINISHES_AND_RAISES(Invalid, end);
}

TYPED_TEST(TypedTestAsyncTaskGroup, TaskFactoryFails) {
  TypeParam task_group;
  ASSERT_RAISES(Invalid, task_group.AddTask([] { return Status::Invalid("XYZ"); }));
  ASSERT_RAISES(Invalid, task_group.AddTask([] { return Future<>::Make(); }));
  ASSERT_FINISHES_AND_RAISES(Invalid, task_group.End());
}

TYPED_TEST(TypedTestAsyncTaskGroup, AddAfterFailed) {
  TypeParam task_group;
  ASSERT_RAISES(Invalid, task_group.AddTask([] {
    return Future<>::MakeFinished(Status::Invalid("XYZ"));
  }));
  ASSERT_RAISES(Invalid, task_group.AddTask([] { return Future<>::Make(); }));
  ASSERT_FINISHES_AND_RAISES(Invalid, task_group.End());
}

TYPED_TEST(TypedTestAsyncTaskGroup, Stress) {
  constexpr int NTASKS = 100;
  TypeParam task_group;
  std::vector<std::thread> threads;
  for (int i = 0; i < NTASKS; i++) {
    ASSERT_OK(task_group.AddTask([&threads] {
      Future<> fut = Future<>::Make();
      threads.emplace_back([fut]() mutable { fut.MarkFinished(); });
      return fut;
    }));
  }
  ASSERT_FINISHES_OK(task_group.End());
  for (auto& thread : threads) {
    thread.join();
  }
}

TEST(StandardAsyncTaskGroup, TaskFinishesAfterError) {
  AsyncTaskGroup task_group;
  Future<> fut1 = Future<>::Make();
  ASSERT_OK(task_group.AddTask([fut1] { return fut1; }));
  ASSERT_RAISES(Invalid, task_group.AddTask([] {
    return Future<>::MakeFinished(Status::Invalid("XYZ"));
  }));
  Future<> finished_fut = task_group.End();
  AssertNotFinished(finished_fut);
  fut1.MarkFinished();
  ASSERT_FINISHES_AND_RAISES(Invalid, finished_fut);
}

TEST(StandardAsyncTaskGroup, FailAfterAdd) {
  AsyncTaskGroup task_group;
  Future<> will_fail = Future<>::Make();
  ASSERT_OK(task_group.AddTask([will_fail] { return will_fail; }));
  Future<> added_later_and_passes = Future<>::Make();
  ASSERT_OK(
      task_group.AddTask([added_later_and_passes] { return added_later_and_passes; }));
  will_fail.MarkFinished(Status::Invalid("XYZ"));
  ASSERT_RAISES(Invalid, task_group.AddTask([] { return Future<>::Make(); }));
  Future<> finished_fut = task_group.End();
  AssertNotFinished(finished_fut);
  added_later_and_passes.MarkFinished();
  AssertFinished(finished_fut);
  ASSERT_FINISHES_AND_RAISES(Invalid, finished_fut);
}

// The serialized task group can never really get into a "fail after add" scenario
// because there is no parallelism.  So the behavior is a little unique in these scenarios

TEST(SerializedAsyncTaskGroup, TaskFinishesAfterError) {
  SerializedAsyncTaskGroup task_group;
  Future<> fut1 = Future<>::Make();
  ASSERT_OK(task_group.AddTask([fut1] { return fut1; }));
  ASSERT_OK(
      task_group.AddTask([] { return Future<>::MakeFinished(Status::Invalid("XYZ")); }));
  Future<> finished_fut = task_group.End();
  AssertNotFinished(finished_fut);
  fut1.MarkFinished();
  ASSERT_FINISHES_AND_RAISES(Invalid, finished_fut);
}

TEST(SerializedAsyncTaskGroup, FailAfterAdd) {
  SerializedAsyncTaskGroup task_group;
  Future<> will_fail = Future<>::Make();
  ASSERT_OK(task_group.AddTask([will_fail] { return will_fail; }));
  Future<> added_later_and_passes = Future<>::Make();
  bool added_later_and_passes_created = false;
  ASSERT_OK(task_group.AddTask([added_later_and_passes, &added_later_and_passes_created] {
    added_later_and_passes_created = true;
    return added_later_and_passes;
  }));
  will_fail.MarkFinished(Status::Invalid("XYZ"));
  ASSERT_RAISES(Invalid, task_group.AddTask([] { return Future<>::Make(); }));
  ASSERT_FINISHES_AND_RAISES(Invalid, task_group.End());
  ASSERT_FALSE(added_later_and_passes_created);
}

TEST(SerializedAsyncTaskGroup, Abort) {
  SerializedAsyncTaskGroup task_group;
  struct Task {
    bool started = false;
    Future<> finished = Future<>::Make();
  };
  auto task_factory = [](Task& task) -> std::function<Future<>()> {
    return [&task] {
      task.started = true;
      return task.finished;
    };
  };
  Task one, two;
  ASSERT_OK(task_group.AddTask(task_factory(one)));
  ASSERT_OK(task_group.AddTask(task_factory(two)));
  Future<> group_done = task_group.OnFinished();
  AssertNotFinished(group_done);
  ASSERT_TRUE(one.started);
  ASSERT_FALSE(two.started);
  Future<> abort_done = task_group.Abort(Status::Invalid("XYZ"));
  AssertNotFinished(abort_done);
  one.finished.MarkFinished();
  ASSERT_FINISHES_AND_RAISES(Invalid, group_done);
  ASSERT_FINISHES_AND_RAISES(Invalid, abort_done);
  ASSERT_FALSE(two.started);
}

}  // namespace util
}  // namespace arrow
