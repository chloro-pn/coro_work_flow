#include "series_work_awaiter.h"

#include <condition_variable>
#include <mutex>

#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "gtest/gtest.h"
#include "workflow/WFTaskFactory.h"

using coro_work_flow::start_series_work;

TEST(series_work_awaiter, basic) {
  async_simple::executors::SimpleExecutor ex(2);

  auto task1 = WFTaskFactory::create_go_task("test", []() {});
  task1->set_callback([](WFGoTask *task) { series_of(task)->set_context(new int(0)); });

  auto task2 = WFTaskFactory::create_go_task("test", []() {});
  task2->set_callback([](WFGoTask *task) {
    int *i = static_cast<int *>(series_of(task)->get_context());
    *i += 1;
  });

  auto lazy_task = [&]() -> async_simple::coro::Lazy<int> {
    void *context = co_await start_series_work(task1, task2);
    int ret = *static_cast<int *>(context);
    delete (int *)context;
    co_return ret;
  };

  int result{0};
  std::condition_variable cv;
  std::mutex mut;
  bool done = false;
  lazy_task().via(&ex).start([&](auto &&v) {
    result = v.value();
    std::unique_lock<std::mutex> l(mut);
    done = true;
    cv.notify_one();
  });
  std::unique_lock<std::mutex> l(mut);
  cv.wait(l, [&] { return done == true; });
  EXPECT_EQ(result, 1);
}
