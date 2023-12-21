#pragma once

#include <cassert>
#include <coroutine>

#include "DnsMessage.h"
#include "HttpMessage.h"
#include "MySQLMessage.h"
#include "RedisMessage.h"
#include "async_simple/Executor.h"
#include "workflow/WFTaskFactory.h"

namespace coro_work_flow {

namespace detail {

template <typename RESP>
struct network_response_t {
  network_response_t(int s, int e, RESP&& r) : state(s), error(e), response(std::move(r)) {}

  int state;
  int error;
  RESP response;
};

template <typename REQ, typename RESP>
class NetworkTaskAwaiter {
 public:
  NetworkTaskAwaiter(async_simple::Executor* ex, WFNetworkTask<REQ, RESP>* task)
      : ex_(ex), task_(task), state_(0), error_(0), resp_() {
    assert(ex_ != nullptr);
    assert(task_ != nullptr);
  }

  bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> h) {
    task_->set_callback([h, this](WFNetworkTask<REQ, RESP>* myself) {
      this->state_ = myself->get_state();
      this->error_ = myself->get_error();
      this->resp_ = std::move(*myself->get_resp());
      this->ex_->schedule(h);
    });
    task_->start();
  }

  network_response_t<RESP> await_resume() noexcept { return network_response_t(state_, error_, std::move(resp_)); }

 private:
  async_simple::Executor* ex_;
  WFNetworkTask<REQ, RESP>* task_;
  int state_;
  int error_;
  RESP resp_;
};

template <typename REQ, typename RESP>
class NetworkTaskAwaitable {
 public:
  NetworkTaskAwaitable(WFNetworkTask<REQ, RESP>* task) : task_(task) {}

  auto coAwait(async_simple::Executor* ex) { return NetworkTaskAwaiter<REQ, RESP>{ex, task_}; }

 private:
  WFNetworkTask<REQ, RESP>* task_;
};

}  // namespace detail

template <typename REQ, typename RESP>
inline detail::NetworkTaskAwaitable<REQ, RESP> start_network_request(WFNetworkTask<REQ, RESP>* task) {
  return detail::NetworkTaskAwaitable<REQ, RESP>{task};
}

inline detail::NetworkTaskAwaitable<protocol::HttpRequest, protocol::HttpResponse> start_http_request(
    WFHttpTask* task) {
  return start_network_request<protocol::HttpRequest, protocol::HttpResponse>(task);
}

inline detail::NetworkTaskAwaitable<protocol::RedisRequest, protocol::RedisResponse> start_redis_request(
    WFRedisTask* task) {
  return start_network_request<protocol::RedisRequest, protocol::RedisResponse>(task);
}

inline detail::NetworkTaskAwaitable<protocol::DnsRequest, protocol::DnsResponse> start_dns_request(WFDnsTask* task) {
  return start_network_request<protocol::DnsRequest, protocol::DnsResponse>(task);
}

}  // namespace coro_work_flow