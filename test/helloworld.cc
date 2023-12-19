#include <string>

#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "gtest/gtest.h"
#include "workflow/WFFacilities.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/Workflow.h"

async_simple::coro::Lazy<std::string> GetHelloWorld() { co_return "hello world"; }

TEST(basic, async_simple) { EXPECT_EQ(async_simple::coro::syncAwait(GetHelloWorld()), std::string("hello world")); }

TEST(basic, work_flow) {
  WFFacilities::WaitGroup wg(1);
  auto task = WFTaskFactory::create_go_task("test", []() {});
  auto task2 = WFTaskFactory::create_go_task("test", []() {});
  auto series = Workflow::create_series_work(task, task2, [&](const SeriesWork *) { wg.done(); });
  series->start();
  wg.wait();
  EXPECT_TRUE(true);
}