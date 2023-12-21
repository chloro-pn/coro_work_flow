#pragma once

#include <cassert>
#include <vector>

#include "async_simple/Executor.h"
#include "workflow/Workflow.h"

namespace coro_work_flow {

namespace detail {

class ParallelWorkAwaiter {
 public:
  ParallelWorkAwaiter(ParallelWork* task, async_simple::Executor* ex) : task_(task), ex_(ex), series_context_() {
    assert(task_ != nullptr);
    assert(ex_ != nullptr);
  }

  bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> h) {
    task_->set_callback([h, this](const ParallelWork* myself) {
      for (size_t i = 0; i < myself->size(); ++i) {
        void* context = myself->series_at(i)->get_context();
        this->series_context_.emplace_back(context);
      }
      this->ex_->schedule(h);
    });
    task_->start();
  }

  // state, error
  std::vector<void*> await_resume() noexcept { return std::move(series_context_); }

 private:
  ParallelWork* task_;
  async_simple::Executor* ex_;
  std::vector<void*> series_context_;
};

class ParallelWorkAwaitable {
 public:
  ParallelWorkAwaitable(ParallelWork* task) : task_(task) {}

  auto coAwait(async_simple::Executor* ex) { return ParallelWorkAwaiter{task_, ex}; }

 private:
  ParallelWork* task_;
};

}  // namespace detail

inline detail::ParallelWorkAwaitable start_parallel_work(ParallelWork* task) {
  return detail::ParallelWorkAwaitable{task};
}

}  // namespace coro_work_flow