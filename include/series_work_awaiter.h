#pragma once

#include <cassert>

#include "async_simple/Executor.h"
#include "workflow/Workflow.h"

namespace coro_work_flow {

namespace detail {

class SeriesWorkAwaiter {
 public:
  SeriesWorkAwaiter(async_simple::Executor *ex, SeriesWork *sw) : ex_(ex), sw_(sw), context_(nullptr) {
    assert(ex != nullptr);
    assert(sw_ != nullptr);
  }

  bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> h) {
    sw_->set_callback([h, this](const SeriesWork *myself) {
      this->context_ = myself->get_context();
      ex_->schedule(h);
    });
    sw_->start();
  }

  // 返回值为series的context，如果未设置可以忽略
  void *await_resume() noexcept { return context_; }

 private:
  async_simple::Executor *ex_;
  SeriesWork *sw_;
  void *context_;
};

class SeriesWorkAwaitable {
 public:
  explicit SeriesWorkAwaitable(SeriesWork *sw) : sw_(sw) {}

  auto coAwait(async_simple::Executor *ex) { return SeriesWorkAwaiter(ex, sw_); }

 private:
  SeriesWork *sw_;
};

}  // namespace detail

inline detail::SeriesWorkAwaitable start_series_work(SubTask *first, SubTask *last = nullptr) {
  SeriesWork *sw{nullptr};
  if (last != nullptr) {
    sw = Workflow::create_series_work(first, last, nullptr);
  } else {
    sw = Workflow::create_series_work(first, nullptr);
  }
  return detail::SeriesWorkAwaitable{sw};
}

inline detail::SeriesWorkAwaitable start_series_work(SeriesWork *sw) { return detail::SeriesWorkAwaitable{sw}; }

}  // namespace coro_work_flow