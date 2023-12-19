load('@bazel_tools//tools/build_defs/repo:git.bzl', 'git_repository')
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "com_github_async_simple",
    remote = "https://github.com/chloro-pn/async_simple",
    branch = "dev",
)

load("@com_github_async_simple//bazel/config:deps.bzl", "async_simple_dependencies")
async_simple_dependencies()

git_repository(
    name = "workflow",
    remote = "https://github.com/sogou/workflow",
    tag = "v0.11.1",
)

git_repository(
    name = "googletest",
    remote = "https://ghproxy.com/https://github.com/google/googletest",
    tag = "release-1.11.0",
)