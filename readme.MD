## coro_work_flow

coro_work_flow是Workflow项目的协程组件实现，基于async_simple。

## implementing list
* async_simple_task
  该组件将一个Lazy协程封装为SubTask交由Workflow调度。

* series_work_awaiter
  该组件将start_series_work功能封装为Awaiter，可以在Lazy中co_await。