
- [问题](#问题)
- [设计](#设计)
  - [组件和模块](#组件和模块)
  - [数据流](#数据流)
  - [接口和协议](#接口和协议)
  - [错误码和结构体等定义](#错误码和结构体等定义)
  - [库表设计](#库表设计)
  - [安全性](#安全性)
  - [代码组织](#代码组织)
  - [扩展性](#扩展性)
  - [测试](#测试)
- [编译](#编译)
  - [本地环境](#本地环境)
  - [三方依赖](#三方依赖)
- [操作](#操作)
- [遇到的问题](#遇到的问题)
- [扩展](#扩展)

## 问题

终端登录设计：
* 可注册登录；
* 同时只能在一台设备登录，多端登录时根据管理策略管理设备踢出情况；
* `C++`、`gRPC` 实现，`Bazel` 编译管理依赖，考虑数据、传输等安全


## 设计

### 组件和模块

分三个模块：

1）客户端，发送操作请求(注册，登录，更新，登出)

2）后台管理端，用于数据 `mock`，内部组件 `metrics` 观察，配置项动态变更（如设备踢出等），信息广播和查看用户数据等。后台管理端这些功能也可作测试用。

3）服务端，接收和处理用户和管理端的请求，其中登录后服务端保持与客户端的单向流（`stream`），通过阻塞队列，将变更的消息（更新、踢出、广播等）推送给客户端。

设计图如下：

![arch](https://www.plantuml.com/plantuml/svg/TP9HRiCW38RVEONLzrwWcdQHRK8Kd0ILG0PaqpJrxZEGfD1agAg8VzkOdyzW5o4wyBeV8YZjKKRjB6D2HkTX3kYNhL2ZflWav4tq22VZUcsvD1fjFC4lk--qN74iKTilz4a3Mfqp2ZsSIZC-2AiC-h3AQKdX5SnM-1_kyNF640FRnBK-JSj3z2Z6kZOjAfaHhiR9cxOzpvzRmfsLVzPCCypNkajoHjZU89GJ-2YcYTpPwoE6G7VbdVFVDFQISs-xPtF-l-f8WxOHRSD4HxH1wMod-pRMOypFrWIltWabHOZrB4f4SHM1NcNTrNEyJSwPZwFPvwXD-CSRkdlYPwzCtaN71pJJmn3wfL7XvNn-XHeebksfumM_Qb_4raQu54PWOwHkMYgU8DkL0B7-Rg4Gkhmr8LCcNUWgdLeEdlkcZEJWeR3Qn9eUMThJiulal2UMsy-Z3BOGb3M26vh0IW1BvEL8VcMsqFIupaOI77YGSTmEkC1DKxFHAVm3)


`server` 的主要模块说明如下：

* `Admin Handlers` 处理来自 `admin` 端的请求；
* `Client Handlers` 处理来自用户的请求，其中对服务端到客户端的流，采取了阻塞队列的方式。可考虑使用 `Redis` 的 [BRPOPLPUSH](https://redis.io/commands/brpoplpush) 命令或持久化效果更好的消息队列；
* `Hub` 管理用户的流和消息转发；
* `Utils` 为一些 `token` 生成/解码，`proto` 转换，`mock` 数据生成等方便函数或类；
* `DB` 存储用户注册登录等基本信息，配置了连接池。


操作可参考这里的录屏：

[![asciicast](https://asciinema.org/a/313861.svg)](https://asciinema.org/a/313861)

录屏中面板布局如下：
```
+------------------------------------------------------------------------------+
|                                                                              |
|                                                                              |
|                                  Admin Panel                                 |
|              (mock data, inspect, broadcast, config, getuser)                |
|                                                                              |
|                                                                              |
|------------------------------------------------------------------------------|
|                     |                     |                                  |
|     User1 Login1    |    User1 Login2     |                                  |
|                     |                     |                                  |
|---------------------+---------------------|          Server Panel            |
|                     |                     |                                  |
|     User2 Login1    |    User2 Login2     |                                  |
|                     |                     |                                  |
+------------------------------------------------------------------------------+
```


其中客户端与服务端交互有：

* 注册用户信息(`register`)
* 登录(`login`)，并维持服务端到客户端的单向流，接收信息推送、保活等。推送信息可以是：变更的基本信息、系统广播信息，退出登录信息，在另一终端登录时原登录被踢出的通知信息等等，可根据消息类型灵活添加；用户在另一个终端登录时，会根据服务端配置的踢出策略（`kick_mode`, 1:踢出旧登录，2:保留旧登录，不能新登录）
* 修改基本用户信息(`update`)。此操作会返回成功，新的用户信息会通过服务端与客户端的单向流向客户端更新；
* 登出信息(`logout`)。此消息会在登录流中返回登出通知。

管理后台端与服务端的交互有：

* `Mock` 数据供客户端的各命令测试，由于做了签名校验，手动配置会麻烦，所以后端生成；
* 配置查看与设置（`config`），比如设置踢出模式、`token` 校验等级等；
* 服务端的 `metrics` 相关暴露（`inspect`），比如查询 `Hub` 中活跃流数、连接池的数据情况等；
* 获取用户基本信息（`getuser`）接口，查看操作中数据是否存在或是否更新；
* 广播（`broadcast`），其实也是为了测试流推送情况。

### 数据流

1. 用户与 `client` 端交互时，采用 `命令 JSON包体` 的格式，`client` 解析 `JSON` 包体并转为 `protobuf` 格式， `client` 与 `server` 交互采用 `grpc/protobuf`;
2. 用户与 `admin` 端交互时，采用 `命令 子命令1/数据1 子命令2/数据2 ` 的格式，其中后面两个参数根据具体的命令可选。`admin` 与 `server` 交互采用 `grpc/protobuf`

由于 `Protobuf` 中定义的字段比较多，输入也不便，所以采用输入为 `JSON` 包体、`client` 将其转为 `protobuf` 格式再与 `server` 交互。另外由于做了一些签名校验，手动输入也会不大方便，所以`admin` 端提供了 `client` 端所有命令的即时 `mock` 数据,方便测试。

`client` 和 `admin` 收到 `server` 返回，以及 `server` 收到 `client` 和 `admin` 的请求，都做了转 `JSON` 输出，方便查看。 


### 接口和协议

对 `CommonHeaderReq` 一些说明：

1. 每个请求都会带这个字段。`user_name` 为 16位的 `alphanum`，`token` 是64位，`stamp` 是毫秒级时间戳。
2. 对注册请求，`token = md5(密码)+md5(验证码)`，实验中为方便测试，`mock` 数据生成的密码和验证码对应手机号中的中间四位和后四位。
3. 对非注册请求，`token = md5(src+key)+bin2hex(src)`, 其中 `src` 为 `systemType + timestamp` 组成的16位对其的字符串, `key` 为 `user_name`, 其中 `systemType` 定义见 `defs.hpp`，数值不超过8，`timestamp` 为毫秒级13位，这两字段填充16位没问题。即可以通过 `header` 直接校验 `token` 是否有效。在安全等级较高的情况下，由于 `timestamp` 和 `systemType` 等信息都存储在登录信息表中，也可以通过表中字段合成来校验，免去存缓存。


对注册请求和登录请求，都会有一个签名校验，这个值为前面定义的所有字段的值转为字符串按序拼接成的长串的 `md5` 值。`header` 里面的 `stamp` 一是可以做幂等处理，另外也可和反解后的 `token` 里面的时间做比较，在安全性高的校验中可能会用到。

对 `CommonHeaderResp`，`code` 可参考 `defs.hpp` 中的枚举定义，`message` 为枚举值对应的字符串。

登录成功后的返回为 `stream` 类型的 `LoginResp`，对 `LoginResp`, `messages` 为 `repeated` 类型，可以有多种类型的消息推送，这里做了个抽象，`msg_type` 为可推送的消息类型（用户信息更新，接收广播信息，朋友圈更新，联系人更新，在另一台设备登录后被踢出等），`content` 字段比较灵活，可以是 `JSON` 包体。当然这也是为了实验操作方便。考虑传输效率等，用 `Oneof` 可能会更合适，但这会涉及到每次新增消息类型时逻辑中要添加 `proto` 逻辑适配，相比起来 `JSON` 要方便些。

登录目前只用 `token` 来登录，协议中已定义了 `login_type`，容易扩展，实验中忽略了 `login_type` 的校验，做了签名校验、`token` 有效性校验和用户存在性校验等。

详细 `proto` 设计可见 [./apis/protocol/v1/login.proto](./apis/protocol/v1/login.proto)


### 错误码和结构体等定义

相关的有 消息类型，返回码，设备系统类型、设备类型、渠道类型，登录类型，用户状态还有相关的结构体等等，定义都在 [./apps/login/defs.hpp](./apps/login/defs.hpp) 中。

### 库表设计

共四张表：

* `user_basic_tab` 用户基本信息表，当前更新用户信息操作只会更新此表；
* `user_secure_tab` 用户安全信息表，原则上需要对 `passwd` 做 `bcrypt` 等加密，当前只做 `md5` 处理；
* `user_login_tab` 用户登录信息表，记录了 `login_time` 和 `system_type`，可还原 `token`。
* `user_register_tab` 用户注册信息表，只追加不修改;


用户新注册时，会同时写 `register`, `secure` 和 `basic` 三张表。原则上是一事务。但是依赖的三方库中貌似没找到事务支持。

用户登录时，会添加登录记录，登出或踢出时，会更新登出时间。

`register` 和 `login` 两张表中的 `ip` 字段操作中没有用到。

详细的数据表的 `schema` 可见 [./confs/login_db.sql](./confs/login_db.sql)


### 安全性

由于主要是实验性质，所以并没有依赖安全相关的重操作，但是在设计上，有这方面的考虑。

* `gRPC` 的 `secure credentials`，启动选项里添加，实验中没有添加，添加证书操作可参考[示例](https://github.com/grpc/grpc/issues/9593)，注意证书的生成中的参数（`--days` 和 `CN`）。
* 用户数据表分安全信息和基本信息，安全信息中包含用户密码和手机号，原则上，密码需采用破解难度高的加密库（如`bcrypt`）,同时这个表最好置于另一个访问权限高的数据库中，通过专门的 `RPC` 服务调用。本次操作中，只取了 `md5` 码，同基本信息表置于同一数据库中直接访问数据库得到。
* 用户注册后，会返回 `token`，在随后的操作中，都会带着 `token` 来操作。但是不同的操作对 `token` 的安全等级不尽相同，因此对同一 `token` 也可能有不同的等级的鉴定。理想的方式是加密了些关键参数（如时间戳、系统类型），在需要调整安全等级时，虽然数据库没存储具体值，但是可以根据相关字段反解或合成，无需单独存储。（见上面对 `CommonHeaderReq` 的说明）。
* 用户输入校验，对注册和请求做签名校验（见上面说明）。对每个具体字段的长度，大小，格式等，这里没有做校验，后续可以参考 `Envoy` 的 [proto-gen-validate](https://github.com/envoyproxy/protoc-gen-validate)。
* 可考虑添加 `gRPC` 框架中的[健康检查](https://github.com/grpc/grpc/issues/13962)和拦截器，目前这两者处于 `experimental` 状态 ？
* 由于对配置项添加了脱敏处理（`to_json`），打印日志时不会打出具体的 `MySQL` 账号密码等配置。
* `SQL` 操作尽量模版化，用 `prepared statement`，目前所选的依赖包里，`prepared statement` 对查询操作结果 `fetch` 似乎有点问题( `exec` 操作没问题，所以写操作都可以用 `prepared statement`)，所以就直接用了 `query`。不过由于当前的查询参数都是`userName` 或 `phoneNumber`, 模式容易鉴定，所以问题不大。实验中的 `MySql` 依赖是基于官方接口的现代 `C++` 包装， 生产环境可改成支持 `ORM` 操作的依赖。
* 操作都是线程安全。

### 代码组织


代码组织如下。设计上将 `proto` 文件目录和服务逻辑分离，并没有按照 `bazel` 教程将 `proto` 文件和代码放在一起。

代码采用 `C++ 1x` 编写，风格上参考 [`Envoy Proxy`](https://github.com/envoyproxy/envoy)，其中 `TODO` 或 三方依赖修改等用 `slonguo` 注释备注。

`proto` 做了版本的区分。

配置的时候，将 `proto` 文件通过 `protoc` 工具链来生成(`build_proto.sh`)，并且将生成的代码文件一起上库，这样操作主要是考虑到协议变更不频繁，对应语言的代码也一目了然，方便编辑器跳转。

`apps` 目录下，`utils` 和 `3rd` 为公用依赖，除了 `login`，其他的一些新增服务模块也可置于 `apps` 下。

执行文件的 `main` 入口文件直接置于 `apps` 下，如当前 `login_client.cc`, `login_admin.cc` 和 `login_server.cc`。由于目前接口不多，所以也将具体执行类和 `main` 置于同一文件中。随着接口和逻辑复杂性的提高，应单独提取出执行类到具体的业务目录下。

```text
.
├── Makefile                          # 对 bazel 构建和清理等命令的简单包装
├── README.md                         # 本文档
├── WORKSPACE                         # bazel 的 workspace
├── apis                              # proto 协议和其生成文件目录
│   ├── BUILD                         # apis 的 build 文件，供 apps 依赖
│   ├── cpp                           # 以下是根据协议生成的 cpp 代码
│   │   └── protocol
│   │       └── v1
│   │           ├── login.grpc.pb.cc
│   │           ├── login.grpc.pb.h
│   │           ├── login.pb.cc
│   │           └── login.pb.h
│   └── protocol                      # 协议目录，按版本分
│       ├── v1                        # v1 为实验中用到的 proto
│       │   └── login.proto
│       └── v2                        # v2 加了 proto-gen-validate 和 grpc-gateway-swagger
│           └── login.proto           #    的 proto 格式，只是 demo, 没有用到
├── apps                              # 服务代码主目录
│   ├── 3rd                           # 三方依赖，由于多是单文件，所以放在一起
│   │   ├── bin2ascii.hpp             #    十六进制和字符串间的转换
│   │   ├── json.hpp                  #    json 处理相关
│   │   ├── loguru.cc                 #    日志
│   │   ├── loguru.hpp
│   │   ├── magic_enum.hpp            #    枚举转字符串
│   │   ├── md5.cc                    #    md5
│   │   ├── md5.hpp             
│   │   └── sql                       #    mysql 封装接口，做了一些改动（见后文）
│   │       ├── mysqlplus.h
│   │       └── (sub dirs omitted)
│   ├── BUILD                         # 服务端 build 文件，依赖了 apis 的build
│   ├── login                         # 登录的一些功能性模块
│   │   ├── admin_handlers.cc         # admin handlers 处理 admin 的请求
│   │   ├── admin_handlers.hpp
│   │   ├── config.cc                 # 配置相关，对象可全 JSON 化
│   │   ├── config.hpp
│   │   ├── dao.cc                    # DB 连接 相关
│   │   ├── dao.hpp
│   │   ├── defs.hpp                  # 结构体，错误码，枚举 等定义
│   │   ├── hub.cc                    # stream 管理，消息转发
│   │   ├── hub.hpp
│   │   ├── proto_helper.cc           # 对象的 proto 和 JSON 协议转换助手
│   │   ├── proto_helper.hpp
│   │   ├── session.hpp               # token 编解码
│   │   └── stream.hpp                # stream 对象
│   ├── login_admin.cc                # admin app的主逻辑
│   ├── login_client.cc               # client app的主逻辑
│   ├── login_server.cc               # server app的主逻辑
│   ├── tests                         # 测试相关(子目录里都是.cc文件，这里略去)
│   │   ├── xxx.cc(omitted)           # utils 和 3rd 关联的测试
│   └── utils                         # 工具箱
│       ├── blocking_queue.hpp        #   阻塞队列，线程安全
│       ├── conn_options_jsonify.cc   #   对三方mysql依赖的连接选项的 json 对象化包装
│       ├── conn_options_jsonify.hpp
│       ├── conn_pool.hpp             #   连接池，线程安全
│       ├── strings.cc                #   常用字符串处理函数
│       ├── strings.hpp
│       └── time.hpp                  #   时间相关函数
├── build_proto.sh                    # proto 文件处理脚本
├── confs                             # 配置
│   ├── docker-compose.yaml           #    服务的依赖组件，这里是 mysql
│   ├── login_db.sql                  #    库表结构
│   └── server.json                   #    server 配置
└── mysql.BUILD                       # mysql的build，依赖处理

```

### 扩展性

由于使用了 `gRPC`, 代码全部使用现代 `C++`，没有调用系统特有 `API`，原则上可以跨平台，只需修改 `Bazel` 的编译项即可（主要是 `MySQL` 依赖）。

字段枚举、接口、模式等方面都考虑到了可扩展性，基本不会影响框架改动。

### 测试

* 功能性测试
  * `apps/tests` 下的测试，主要是针对一些三方依赖(`3rd`)和工具箱(`utils`)的通用接口和使用方法测试
  * `login-admin` 设计出来也是为了方便对外 `apis` 接口的测试，`login-client` 中可将 `login-admin` `mock` 出来的数据手动修改测试（注意同时修改 `sign` 值）
* 性能测试：实验中没有要求，暂无添加。由于有了 `login-admin` 的数据 `mock`，原则上可以很好的批量生成数据进行并发性能测试。



## 编译

### 本地环境
```
MacOS 10.15.3 (19D76)

clang：Apple clang version 11.0.0 (clang-1100.0.33.17)

Bazel: 2.2.0, 编译时需加选项 `--cxxopt='-std=c++17'`
```

对编译做了个简单的 `Makefile` 包装：

```makefile
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
```

注意到由于上面提到的代码组织的模式，对 `proto` 文件需要本地编译（`build_proto.sh`），这就需要本地编译安装 `protoc` 和关联的 `grpc_cpp_plugin` 插件。如果需要协议字段检查或 `HTTP` 转 `gRPC` 或根据协议生成 `Swagger` 文档注释，需要另安装类似 `protoc-gen-validate` 和 `grpc-gateway` 生态相关的插件（这两个目前对 `Golang` 和 `Python` 支持较好，对 `C++` 支持不是很完善）（`proto` 的编译环境可以 `docker` 化）。

本实验中由于已上传了相关的 `proto` 生成文件，所以不需依赖 `protoc` 链，直接 `make build` 即可。

### 三方依赖

设计上尽量减少依赖或尽量使用轻量级依赖。除了`gRPC` 和 `protobuf`，还有以下依赖：

* [JSON](https://github.com/nlohmann/json) , 对自定义结构体的序列化和反序列化处理非常方便（自定义 `to_json` 和 `from_json`）

* [MD5](https://github.com/joyeecheung/md5)，`MD5` 处理

* [loguru](https://github.com/emilk/loguru)，日志库

* [mysql modern cpp](https://github.com/daotrungkien/mysql-modern-cpp) `C++1x` 风格的 `MySQL` 包装。
  
由于这几个依赖都轻量，所以直接复制到本地的 `3rd` 目录下并做了相应调整。没有用到重一些的加解密库，没有依赖缓存等组件。

注意到 `mysql modern cpp` 只是基于官方的 `mysql.h` 做了一个现代 `C++` 的包装，并不是 `ORM`, 所以存在诸多问题。实际操作中应尽量选择 `ORM`。
在使用 `mysql modern cpp` 过程中，出现或调整了以下几个问题：

* 没有事务支持（对应注册用户时写三张表的操作），这个暂时没解决
* 编译时开启了 `NO_STD_OPTIONAL`，虽然实验中已开启 `c++17` 参数，但编译这个依赖的时候还是会有大量报错，所以开启了这个宏
* 添加了对 `connect_options` 的 `JSON` 序列化/反序列化处理（`conn_options_jsonify.hpp`）
* 头文件依赖调整
* `MySQL` 8.x 中移除了 `my_bool` 类型，这里做了修改
* 编译时 `std::move` 的调整
* `prepared_statement` 预处理时查询结果 `fetch` 似乎有问题，所以查询用了 `query` 接口，但是这里难防注入(目前通过参数校验来处理)。整体看，待换成 `ORM` 式的依赖

## 操作


首先需要配置对应的数据库和服务器，配置可见 `confs` 目录。对配置文件的说明如下：

配置里的对象全部可以 `JSON` 对象序列化和反序列化，关联的 `JSON` 字段见 `defs.hpp` 和 `connect_options_jsonify.hpp`，除了 `DB` 连接选项外，其他字段都是必选字段。

```json
{
    // server 配置
    "server": {
        "endpoint": "127.0.0.1:12345"
    },

    // admin 配置，这里可以动态更新，比如多终端登录时踢出模式配置，token 校验等级配置等
    "admin": {
        "kick_mode": 1
    },

    // db 配置，db 连接选项做了 json 序列化/反序列化
    "db": {
        "conn":{
            "server": "127.0.0.1",
            "port": 3306,
            "username": "root",
            "password": "passwd",
            "dbname": "login_db"
        },
        "pool_size": 10
    },

    // log 配置
    "log": {
        "file_name": "login.log",
        "file_mode": 0,
        "level": 0,
        "verbosity": 1
    }
}
```

操作可以参考前面的录屏示例。涵盖了所有的操作。 `login-client` 和 `login-admin` 默认地址取 `127.0.0.1:12345`，之后直接输入 `help` 查看所支持的操作，连接的时候需要注意看环境变量中是否有 `http_proxy`，如果有需要清理一下。

注意到：`register` 成功后，会返回一个 `token`。原则上，后续的操作都需要带这个 `token` 来校验。但实验中为了方便，没有取数据库里登录记录中的登录时间和系统类型，而是通过 `mock` 操作取当前时间和随机系统类型生成了 `token` 来校验。这就关联到了上面安全里说的 `token` 根据配置的安全校验等级来判断是否需要深度校验的问题。测试中跳过了这一环节，实操中可以在配置里的 `admin` 下添加一个 `token_check_level` 选项来控制。另外这里的 `token` 编码比较简单，后半部分可以直接看出来是做了转十六进制处理，实际中需要高级些的加解密方式。

`help` 命令：

对 `client`:

```
Usage: cmd json_data
cmds={login,register,update,logout}, use admin tool to generate JSON data.
```

对 `admin`：

```
--------------------------------------------------------------------------------
| command |   sub command/data  |                   descriptions               |
--------------------------------------------------------------------------------
|broadcast msg                   broadcast a message to test grpc stream.      |
--------------------------------------------------------------------------------
|config                                                                        |
|          set    json_config    eg:config set {"kick_mode":1}                 |
|          show                  show config info(db user/passwd obscured)     |
--------------------------------------------------------------------------------
|exit      (no args)             exit program. `exit` can also be `quit`, `bye`|
--------------------------------------------------------------------------------
|getuser   user_name             get user basic info.                          |
--------------------------------------------------------------------------------
|help                                                                          |
|          (no args)             `help` can also be `man`, `info`              |
|          specific_cmd          help info on specific command                 |
--------------------------------------------------------------------------------
|inspect   (no args)             show internal stats                           |
--------------------------------------------------------------------------------
|mock                                                                          |
|          login    user_name    eg: mock login minkovsky                      |
|          logout   user_name    eg: mock logout alexia                        |
|          register user_name    eg: mock register bob                         |
|          update   user_name    eg: mock update isaac                         |
--------------------------------------------------------------------------------
```


## 遇到的问题

* 运行时，需关闭本地代理([issue](https://github.com/grpc/grpc/issues/9989))；
* 添加 `mysql` 的 `mysqlclient` 依赖在 `Mac` 下一直添加不上，可能是当前的 `MacOS` 下系统包含路径不对，通过这里的方法来[解决](https://stackoverflow.com/questions/58908156/why-bazel-not-search-system-include-paths)；
* `MacOS` 下用 `std::chrono` 相关的接口(`time_since_epoch`)获取当前时间戳，似乎有点[问题](https://stackoverflow.com/questions/45882606/why-does-clang-g-not-giving-correct-microseconds-output-for-chronohigh-res?noredirect=1&lq=1)（`time.hpp`）,不过这个不影响实验；
* `protobuf` 的 `MessageToJsonString` 函数在转换 `proto` 为 `JSON` 时，`uint64` 类型的值会被[转为字符串](https://github.com/protocolbuffers/protobuf/issues/2679)；
* `protoc` 本地生成的 `cpp` 文件和 `Bazel` 里生成的可能会因为版本不一致出现不兼容[问题](https://github.com/protocolbuffers/protobuf/issues/7137
)。

## 扩展

1. 阻塞队列替换为持久化等效果更好的常用的中间件；
2. 消息较多时，批量推送（`login.proto`的定义中消息已是`repeated`）；
3. （`gRPC`）请求日志打印采用拦截器模式（拦截器目前处于实验状态），健康检查，异步；
4. 请求字段校验用 `Envoy` 的 [Proto-gen-validate](https://github.com/envoyproxy/protoc-gen-validate)
5. `HTTP` 协议转 `gRPC` 和 `Swagger` 文档生成可参考 [grpc-gateway](https://github.com/grpc-ecosystem/grpc-gateway); 目前 `gateway` 官方不直接支持 [`C++`](https://github.com/grpc-ecosystem/grpc-gateway/issues/15)，不过有 `C++` 嵌入 `GO` 的[示例](https://github.com/yugui/grpc-gateway/tree/example/embed/examples/cmd/example-cxx-server)；
6. 编译环境和依赖 `docker` 打包。