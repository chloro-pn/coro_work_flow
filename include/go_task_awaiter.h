#pragma once

#include <cassert>
#include <coroutine>
#include <string>
#include <type_traits>

#include "async_simple/Executor.h"
#include "workflow/WFTaskFactory.h"

namespace coro_work_flow {

namespace detail {

class GoTaskAwaiter {
 public:
  GoTaskAwaiter(async_simple::Executor* ex, WFGoTask* task) : ex_(ex), task_(task) {
    assert(ex_ != nullptr);
    assert(task_ != nullptr);
  }

  bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> h) {
    task_->set_callback([h, this](WFGoTask* myself) {
      this->user_data_ = myself->user_data;
      this->ex_->schedule(h);
    });
    task_->start();
  }

  // user_data
  void* await_resume() noexcept { return user_data_; }

 private:
  async_simple::Executor* ex_;
  WFGoTask* task_;
  void* user_data_;
};

class GoTaskAwaitable {
 public:
  GoTaskAwaitable(WFGoTask* task) : task_(task) {}

  auto coAwait(async_simple::Executor* ex) { return GoTaskAwaiter{ex, task_}; }

 private:
  WFGoTask* task_;
};

}  // namespace detail

// 直接接受一个WFGoTask, 用户可以设置user_data，并由co_await返回
inline detail::GoTaskAwaitable start_go_task(WFGoTask* task) { return detail::GoTaskAwaitable{task}; }

}  // namespace coro_work_flow