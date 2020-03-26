# load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "3rd_md5",
    srcs = ["3rd/md5.cc"],
    hdrs = ["3rd/md5.hpp"],
    visibility = ["//visibility:public"]
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "3rd_json",
    hdrs = ["3rd/json.hpp"],
    visibility = ["//visibility:public"]
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "3rd_magic_enum",
    hdrs = ["3rd/magic_enum.hpp"],
    visibility = ["//visibility:public"]
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "3rd_loguru",
    srcs = ["3rd/loguru.cc"],
    hdrs = ["3rd/loguru.hpp"],
    visibility = ["//visibility:public"]

)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "login_config",
    srcs = ["login/config.cc","utils/conn_options_jsonify.cc"],
    hdrs = ["login/config.hpp", "utils/conn_options_jsonify.hpp"],
    deps = [
        ":3rd_json",
        ":3rd_loguru",
        ":3rd_mysqlplus"
    ],
)


# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "login_session",
    hdrs = ["login/session.hpp"],
    deps = [":utils_strings"],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "login_proto_helper",
    srcs = ["login/proto_helper.cc"],
    hdrs = ["login/proto_helper.hpp", "utils/time.hpp"],
    deps = [
        "//apis:loginpb",
        ":3rd_md5",
        ":login_defs",
        ":login_session",
        ":utils_strings",
    ],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "login_hub",
    srcs = ["login/hub.cc"],
    hdrs = ["login/hub.hpp", "login/stream.hpp"],
    deps = [
        ":login_defs",
        ":utils_blocking_queue",
    ],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "login_admin_handlers",
    srcs = ["login/admin_handlers.cc"],
    hdrs = ["login/admin_handlers.hpp"],
    deps = [
        ":utils_strings",
        ":login_config",
        ":login_defs",
        ":login_proto_helper",
        ":3rd_json",
        ":login_hub",
        ":login_dao",
        "//apis:loginpb",
    ],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "login_defs",
    hdrs = ["login/defs.hpp"],
    deps = [
        ":3rd_json",
        ":3rd_magic_enum",
        "//apis:loginpb",
    ],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "utils_strings",
    srcs = ["utils/strings.cc"],
    hdrs = ["utils/strings.hpp", "3rd/bin2ascii.hpp"],
    deps = [":3rd_md5"]
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "utils_blocking_queue",
    hdrs = ["utils/blocking_queue.hpp"],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "login_dao",
    srcs = ["login/dao.cc"],
    hdrs = ["login/dao.hpp", "utils/conn_pool.hpp", "utils/time.hpp"],
    deps = [
        ":login_defs",
        ":login_config",
        # ":3rd_mysqlplus"
        "//apis:loginpb",
        ":3rd_json",
    ],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "3rd_bin2ascii",
    hdrs = ["3rd/bin2ascii.hpp"],
    visibility = ["//visibility:public"]

)



# # https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library

# # issue when building on MacOS:
# # ProductName:    Mac OS X
# # ProductVersion: 10.15.3
# # BuildVersion:   19D76
# # https://github.com/bazelbuild/bazel/issues/5391
# # https://github.com/bazelbuild/bazel/issues/3860

# # workaround:
# # https://stackoverflow.com/questions/58908156/why-bazel-not-search-system-include-paths

# # caution:
# # mysql 8.0.1 removes my_bool
# # https://github.com/Motion-Project/motion/issues/995

cc_library(
    name = "3rd_mysqlplus",
    hdrs = [
        "3rd/sql/mysqlplus.h", 
        "3rd/sql/polyfill/datetime.h",
        "3rd/sql/polyfill/function_traits.h",
        "3rd/sql/polyfill/optional.hpp"
    ],
    # includes = [
    #     "mysql/mysql.h",
    #     "mysql/errmsg.h"
    # ],
    # copts = ["-I/usr/local/opt/mysql@5.7/include", "-L/usr/local/opt/mysql@5.7/lib"],
    # copts = ["-I/usr/local/include"],

    # linkopts = ["-lmysqlclient"],

    deps = [
        "@usr_local//:mysql_build"
    ],
    visibility = ["//visibility:public"]
)



# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary

cc_binary(
    name = "test-md5",
    srcs = ["tests/md5_test.cc"],
    deps = [":3rd_md5"],
)

cc_binary(
    name = "test-json",
    srcs = ["tests/json_test.cc"],
    deps = [":3rd_json"],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "test-loguru",
    srcs = ["tests/loguru_test.cc"],
    copts = [],
    deps = [":3rd_loguru"],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "test-mysql",
    srcs = ["tests/mysql_test.cc"],
    copts = [],
    deps = [":3rd_mysqlplus"],
)


# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "test-config",
    srcs = ["tests/config_test.cc"],
    copts = [],
    deps = [
        ":login_config",
        ":3rd_json",
        ":3rd_loguru",
        ":3rd_mysqlplus",
    ],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "test-bin2ascii",
    srcs = ["tests/bin2ascii_test.cc"],
    copts = [],
    deps = [
        ":3rd_bin2ascii",
    ],
)


# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "test-strings",
    srcs = ["tests/strings_test.cc"],
    copts = [],
    deps = [":utils_strings"],
)

cc_binary(
    name = "login-server",
    srcs = [
        "login_server.cc", 
        # "utils/blocking_queue.hpp",
        # "login/config.hpp",
        # "login/dao.hpp",
        # "login/defs.hpp",
        # "login/dao.cc",
    ],
    copts = [],
    deps = [
        ":3rd_loguru",
        ":3rd_json",
        ":3rd_mysqlplus",
        ":login_config",
        ":login_dao",
        ":login_admin_handlers",
        ":login_defs",
        ":login_hub",
        ":utils_blocking_queue",
        "@com_github_grpc_grpc//:grpc++",
        "//apis:loginpb",
    ],
)


cc_binary(
    name = "login-client",
    srcs = [
        "login_client.cc", 
    ],
    copts = [],
    deps = [
        ":3rd_json",
        ":utils_strings",
        ":login_defs",
        "@com_github_grpc_grpc//:grpc++",
        "//apis:loginpb",
    ],
)

cc_binary(
    name = "login-admin",
    srcs = [
        "login_admin.cc", 
    ],
    copts = [],
    deps = [
        ":3rd_json",
        ":utils_strings",
        "@com_github_grpc_grpc//:grpc++",
        "//apis:loginpb",
    ],
)