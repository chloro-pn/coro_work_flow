#include "coro_wf_server.h"

#include <chrono>
#include <string>
#include <thread>

#include "async_simple/executors/SimpleExecutor.h"
#include "gtest/gtest.h"
#include "network_task_awaiter.h"
#include "workflow/HttpMessage.h"
#include "workflow/WFFacilities.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFTaskFactory.h"

using namespace coro_work_flow;

class HelloWorldHttpHandler : public WFNetworkHandler<protocol::HttpRequest, protocol::HttpResponse> {
 public:
  HelloWorldHttpHandler(WFHttpTask* task, async_simple::Executor* ex)
      : WFNetworkHandler<protocol::HttpRequest, protocol::HttpResponse>(task, ex) {}

  async_simple::coro::Lazy<void> handle() override {
    auto task = get_network_task();
    auto resp = task->get_resp();

    resp->append_output_body("hello world");
    resp->set_http_version("HTTP/1.1");
    resp->set_status_code("200");
    resp->set_reason_phrase("OK");
    resp->add_header_pair("Content-Type", "text/html");
    resp->add_header_pair("Server", "Sogou WFHttpServer");
    co_return;
  }
};

TEST(coro_wf_server_task, basic) {
  async_simple::executors::SimpleExecutor ex(2);

  WFHttpServer server([&ex](WFHttpTask* task) {
    auto handler = new HelloWorldHttpHandler(task, &ex);
    series_of(task)->push_back(handler);
  });

  EXPECT_EQ(server.start(12345), 0);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  WFFacilities::WaitGroup wg(1);
  int state = 0;
  int error = 0;
  std::string resp_body;
  /*
  WFTaskFactory::create_http_task("http://127.0.0.1:12345", 0, 0, [&](WFHttpTask* task) {
    state = task->get_state();
    error = task->get_error();
    const void* ptr = nullptr;
    size_t len = 0;
    task->get_resp()->get_parsed_body(&ptr, &len);
    resp_body = std::string((const char*)ptr, len);
    wg.done();
  })->start();
  */
  auto request_lazy_task = [&]() -> async_simple::coro::Lazy<void> {
    auto task = WFTaskFactory::create_http_task("http://127.0.0.1:12345", 0, 0, nullptr);
    detail::network_response_t<protocol::HttpResponse> resp = co_await start_http_request(task);
    state = resp.state;
    error = resp.error;
    const void* ptr = nullptr;
    size_t len = 0;
    resp.response.get_parsed_body(&ptr, &len);
    resp_body = std::string((const char*)ptr, len);
    wg.done();
    co_return;
  };

  request_lazy_task().via(&ex).start([](auto&&) {});

  wg.wait();
  EXPECT_EQ(state, WFT_STATE_SUCCESS);
  EXPECT_EQ(error, 0);
  EXPECT_EQ(resp_body, "hello world");
  server.stop();
}