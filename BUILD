package(default_visibility = ["//visibility:public"])

load("@com_github_async_simple//bazel/config:copt.bzl", "ASYNC_SIMPLE_COPTS")

cc_library(
  name = "coro_async_simple",
  includes = [
    "include"
  ],
  hdrs = glob(["include/**/*.h"]),
  deps = [
    "@workflow//:http",
    "@workflow//:redis",
    "@workflow//:mysql",
    "@com_github_async_simple//:async_simple",
  ]
)

cc_test(
  name = "test",
  srcs = glob(["test/*.cc", "test/*.h"]),
  includes = ["test"],
  copts = ASYNC_SIMPLE_COPTS,
  deps = [
    ":coro_async_simple",
    "@googletest//:gtest",
    "@googletest//:gtest_main",
    "@com_github_async_simple//:simple_executors",
    "@workflow//:http",
  ]
)