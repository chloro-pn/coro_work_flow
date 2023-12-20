#pragma once

#include <cassert>
#include <ctime>
#include <string>
#include <tuple>

#include "async_simple/Executor.h"
#include "workflow/WFTaskFactory.h"

namespace coro_work_flow {

namespace detail {

class TimerTaskAwaiter {
 public:
  TimerTaskAwaiter(async_simple::Executor* ex, WFTimerTask* task) : ex_(ex), task_(task), state_(0), error_(0) {
    assert(ex_ != nullptr);
    assert(task_ != nullptr);
  }

  bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> h) {
    task_->set_callback([h, this](WFTimerTask* myself) {
      this->state_ = myself->get_state();
      this->error_ = myself->get_error();
      this->ex_->schedule(h);
    });
    task_->start();
  }

  // state, error
  std::tuple<int, int> await_resume() noexcept { return std::make_tuple(state_, error_); }

 private:
  async_simple::Executor* ex_;
  WFTimerTask* task_;
  int state_;
  int error_;
};

class TimerTaskAwaitable {
 public:
  TimerTaskAwaitable(WFTimerTask* task) : task_(task) {}

  auto coAwait(async_simple::Executor* ex) { return TimerTaskAwaiter{ex, task_}; }

 private:
  WFTimerTask* task_;
};

}  // namespace detail

inline detail::TimerTaskAwaitable start_timer_task(const std::string& name, time_t seconds, long nanoseconds) {
  WFTimerTask* task = WFTaskFactory::create_timer_task(name, seconds, nanoseconds, nullptr);
  return detail::TimerTaskAwaitable{task};
}

inline detail::TimerTaskAwaitable start_timer_task(time_t seconds, long nanoseconds) {
  WFTimerTask* task = WFTaskFactory::create_timer_task(seconds, nanoseconds, nullptr);
  return detail::TimerTaskAwaitable{task};
}

}  // namespace coro_work_flow