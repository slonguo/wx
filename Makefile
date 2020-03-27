

.PHONY: proto apis build all clean

proto:
	./build_proto.sh

apis:
	bazel build //apis:all

build:
	bazel build --cxxopt='-std=c++17' //apps:all

all: proto build

clean:
	rm -rf ./apis/cpp/*
	rm -f *.log
	bazel clean
