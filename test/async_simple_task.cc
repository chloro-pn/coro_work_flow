#include "async_simple_task.h"

#include <condition_variable>
#include <mutex>

#include "async_simple/executors/SimpleExecutor.h"
#include "gtest/gtest.h"

class TestAsyncSimpleTask : public coro_work_flow::AsyncSimpleTask<int> {
 public:
  TestAsyncSimpleTask(async_simple::Executor *ex) : AsyncSimpleTask<int>(ex) {}

  virtual async_simple::coro::Lazy<int> execute() override { co_return 10; }

 private:
};

TEST(async_simple_task, basic) {
  async_simple::executors::SimpleExecutor ex(2);
  int i = 0;
  auto task = new TestAsyncSimpleTask(&ex);
  std::condition_variable cv;
  std::mutex mut;
  bool done = false;
  task->set_callback([&](async_simple::Try<int> v) {
    i = v.value();
    mut.lock();
    done = true;
    mut.unlock();
    cv.notify_all();
  });
  task->start();
  std::unique_lock<std::mutex> lock(mut);
  while (!done) cv.wait(lock);

  lock.unlock();
  EXPECT_EQ(i, 10);
}