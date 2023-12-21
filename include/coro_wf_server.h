#pragma once

#include <cassert>

#include "async_simple/Executor.h"
#include "async_simple/Try.h"
#include "async_simple/coro/Lazy.h"
#include "workflow/SubTask.h"
#include "workflow/WFServer.h"

namespace coro_work_flow {

template <typename REQ, typename RESP>
class WFNetworkHandler : public SubTask {
 public:
  WFNetworkHandler(WFNetworkTask<REQ, RESP>* task, async_simple::Executor* ex) : task_(task), ex_(ex) {
    assert(task_ != nullptr);
    assert(task_ != nullptr);
  }

  virtual async_simple::coro::Lazy<void> handle() = 0;

  // 默认行为如果有异常的话抛出异常
  virtual void handle_exception(async_simple::Try<void> ret) { ret.value(); }

  virtual void dispatch() override {
    handle().via(ex_).start([this](async_simple::Try<void> ret) {
      this->handle_exception(std::move(ret));
      this->subtask_done();
    });
  }

 protected:
  WFNetworkTask<REQ, RESP>* get_network_task() { return task_; }

 private:
  WFNetworkTask<REQ, RESP>* task_;
  async_simple::Executor* ex_;

  virtual SubTask* done() override {
    auto series = series_of(this);

    delete this;
    return series->pop();
  }
};

}  // namespace coro_work_flow