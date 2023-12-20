#include "timer_task_awaiter.h"

#include <condition_variable>
#include <mutex>

#include "async_simple/coro/Lazy.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "gtest/gtest.h"

using coro_work_flow::start_timer_task;

TEST(timer_task_awaiter, non_name_timer_task) {
  async_simple::executors::SimpleExecutor ex(2);
  auto lazy_task = []() -> async_simple::coro::Lazy<std::tuple<int, int>> {
    co_return co_await start_timer_task(3, 0);
  };

  std::condition_variable cv;
  std::mutex mut;
  bool done = false;

  std::tuple<int, int> result{0, 0};
  lazy_task().via(&ex).start([&](auto&& ret) {
    result = ret.value();
    std::unique_lock<std::mutex> l(mut);
    done = true;
    cv.notify_one();
  });

  std::unique_lock<std::mutex> l(mut);
  cv.wait(l, [&]() { return done == true; });
  EXPECT_EQ(std::get<0>(result), WFT_STATE_SUCCESS);
  EXPECT_EQ(std::get<1>(result), 0);
}