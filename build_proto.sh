#!/bin/bash

# do_build_v2() {
#     cwd=`pwd`
#     cd ./apis
#     mkdir -p cpp golang docs
#     cur_dir=`pwd`

#     src_dir="protocol/v2"
#     cpp_dir=${cur_dir}/cpp/${src_dir}
#     mkdir -p ${cpp_dir}

#     cd ${src_dir}


#     protoc \
#     -I . \
#     -I ${GOPATH}/src \
#     -I ${GOPATH}/src/github.com/envoyproxy/protoc-gen-validate \
#     -I ${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway \
#     -I ${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis \
#     --grpc_out="${cpp_dir}" \
#     --plugin="protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin" \    
#     --go_out="plugins=grpc:${cur_dir}/golang" \
#     --cpp_out="${cpp_dir}" \
#     --validate_out="lang=go:${cur_dir}/golang" \
#     --validate_out="lang=cc:${cpp_dir}" \
#     --grpc-gateway_out=logtostderr=true:${cur_dir}/golang \
#     --swagger_out=logtostderr=true:${cur_dir}/docs \ 
#     ./*.proto
    
#     # protocol/v2/*.proto
# }

do_build_v1() {
    cwd=`pwd`
    cd ./apis
    cur_dir=`pwd`
    
    src_dir="protocol/v1"
    cpp_dir=${cur_dir}/cpp/${src_dir}
    mkdir -p ${cpp_dir}

    cd ${src_dir}
    protoc \
    -I. \
    --grpc_out="${cpp_dir}" \
    --plugin="protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin" \
    --cpp_out="${cpp_dir}" \
    ./*.proto

    cd ${cwd}
}

do_build_v1

# do_build_v2