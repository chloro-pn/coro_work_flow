## coro_work_flow

coro_work_flow是Workflow项目的协程组件实现，基于async_simple。

## implementing list
* async_simple_task
  该组件将一个Lazy协程封装为SubTask交由Workflow调度。

* series_work_awaiter
  该组件将create/start_series_work功能封装为Awaiter，可以在Lazy中co_await。

* start_timer_task
  该组件将create_timer_task功能封装为Awaiter，可以在Lazy中co_await。

* start_parallel_work

* start_go_task

* WFNetworkHandler
  该组件可以将server的处理逻辑封装为Lazy协程处理，见coro_wf_server_test.cc