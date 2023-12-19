#pragma once

#include <functional>

#include "SubTask.h"
#include "Workflow.h"
#include "async_simple/Executor.h"
#include "async_simple/Try.h"
#include "async_simple/coro/Lazy.h"

namespace coro_work_flow {

template <typename RetType>
class AsyncSimpleTask : public SubTask {
 public:
  AsyncSimpleTask(async_simple::Executor *ex) : ex_(ex) {}

  void start() { Workflow::start_series_work(this, nullptr); }

  virtual void dispatch() override {
    execute().via(ex_).start([this](async_simple::Try<RetType> ret) {
      if (this->callback_) {
        this->callback_(std::move(ret));
      }
      this->subtask_done();
    });
  }

  virtual async_simple::coro::Lazy<RetType> execute() = 0;

  using async_simple_callback_t = std::function<void(async_simple::Try<RetType>)>;

  void set_callback(async_simple_callback_t cb) { callback_ = std::move(cb); }

 private:
  async_simple::Executor *ex_;
  async_simple_callback_t callback_;

  virtual SubTask *done() override {
    auto series = series_of(this);

    delete this;
    return series->pop();
  }
};

}  // namespace coro_work_flow