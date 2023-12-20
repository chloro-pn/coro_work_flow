#include "timer_task_awaiter.h"

#include <cerrno>

#include "async_simple/coro/Lazy.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "gtest/gtest.h"
#include "workflow/WFFacilities.h"
#include "workflow/WFTaskFactory.h"

using coro_work_flow::start_timer_task;

TEST(timer_task_awaiter, non_name_timer_task) {
  async_simple::executors::SimpleExecutor ex(2);
  auto lazy_task = []() -> async_simple::coro::Lazy<std::tuple<int, int>> {
    co_return co_await start_timer_task(3, 0);
  };

  WFFacilities::WaitGroup wg(1);

  std::tuple<int, int> result{0, 0};
  lazy_task().via(&ex).start([&](auto&& ret) {
    result = ret.value();
    wg.done();
  });

  wg.wait();
  EXPECT_EQ(std::get<0>(result), WFT_STATE_SUCCESS);
  EXPECT_EQ(std::get<1>(result), 0);
}

TEST(timer_task_awaiter, cancal_name_timer_task) {
  async_simple::executors::SimpleExecutor ex(2);
  auto lazy_task = []() -> async_simple::coro::Lazy<std::tuple<int, int>> {
    co_return co_await start_timer_task("test_timer", 3, 0);
  };

  std::tuple<int, int> result{0, 0};
  WFFacilities::WaitGroup wg(1);

  lazy_task().via(&ex).start([&](auto&& ret) {
    result = ret.value();
    wg.done();
  });
  // 在1s之后发出cancel操作，如果立即发出可能test_timer还没有创建，导致cancel无效
  WFTaskFactory::create_timer_task(1, 0, [](WFTimerTask* self) {
    WFTaskFactory::cancel_by_name("test_timer");
  })->start();
  wg.wait();
  EXPECT_EQ(std::get<0>(result), WFT_STATE_SYS_ERROR);
  EXPECT_EQ(std::get<1>(result), ECANCELED);
}