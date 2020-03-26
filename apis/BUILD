
cc_library(
    name = "loginpb",
    srcs = [
        'cpp/protocol/v1/login.grpc.pb.cc', 
        'cpp/protocol/v1/login.pb.cc', 
    ],
    hdrs = [
        'cpp/protocol/v1/login.grpc.pb.h',
        'cpp/protocol/v1/login.pb.h',
    ],
    deps = [
        '@com_google_protobuf//:protobuf',
        '@com_github_grpc_grpc//:grpc++',
    ],
    visibility = ["//visibility:public"],
)