
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_proto",
    strip_prefix = "rules_proto-218ffa7dfa5408492dc86c01ee637614f8695c45",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/218ffa7dfa5408492dc86c01ee637614f8695c45.tar.gz",
        "https://github.com/bazelbuild/rules_proto/archive/218ffa7dfa5408492dc86c01ee637614f8695c45.tar.gz",
    ],
)
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")
rules_proto_dependencies()
rules_proto_toolchains()


# grpc tag 1.27.3
http_archive(
    name = "com_github_grpc_grpc",
    urls = [
        "https://github.com/grpc/grpc/archive/e73882dc0fcedab1ffe789e44ed6254819639ce3.tar.gz",
    ],
    strip_prefix = "grpc-e73882dc0fcedab1ffe789e44ed6254819639ce3",
)



load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

# resove issue: Unable to find package for @build_bazel_rules_swift//swift:swift.bzl: The repository '@build_bazel_rules_swift' could not be resolved.
# https://github.com/grpc/grpc/issues/20042
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
grpc_extra_deps()

new_local_repository(
    name = "usr_local",
    path = "/usr/local",
    build_file = "mysql.BUILD"
)