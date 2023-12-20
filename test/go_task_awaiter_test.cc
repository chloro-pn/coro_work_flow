#include "go_task_awaiter.h"

#include "async_simple/coro/Lazy.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "gtest/gtest.h"
#include "workflow/WFFacilities.h"
#include "workflow/WFTaskFactory.h"

using coro_work_flow::start_go_task;

TEST(timer_task_awaiter, basic) {
  async_simple::executors::SimpleExecutor ex(2);
  auto lazy_task = []() -> async_simple::coro::Lazy<int> {
    auto task = WFTaskFactory::create_go_task("test", []() {});
    task->user_data = new int(10);
    void* tmp = co_await start_go_task(task);
    int result = *static_cast<int*>(tmp);
    delete (int*)tmp;
    co_return result;
  };

  WFFacilities::WaitGroup wg(1);

  int result = 0;
  lazy_task().via(&ex).start([&](auto&& ret) {
    result = ret.value();
    wg.done();
  });

  wg.wait();
  EXPECT_EQ(result, 10);
}