
<!-- TOC -->

## 问题

终端登录设计：1）可注册登录；2） 同时只能在一台设备登录，根据管理策略管理设备踢出情况；3）C++、gRPC实现，bazel编译管理依赖，考虑数据、传输等安全



## 设计



分三个模块：

1）客户端，发送操作请求(注册，登录，更新，登出)

2）后台管理端，做数据 `mock`，内部组件数据观察，配置项动态变更（如设备踢出等），信息广播和查看用户数据等

3）服务端，接收和处理用户和管理端的请求

设计图如下：

![arch](https://www.plantuml.com/plantuml/svg/TP9HRiCW38RVEONLzrwWcdQHRK8Kd0ILG0PaqpJrxZEGfD1agAg8VzkOdyzW5o4wyBeV8YZjKKRjB6D2HkTX3kYNhL2ZflWav4tq22VZUcsvD1fjFC4lk--qN74iKTilz4a3Mfqp2ZsSIZC-2AiC-h3AQKdX5SnM-1_kyNF640FRnBK-JSj3z2Z6kZOjAfaHhiR9cxOzpvzRmfsLVzPCCypNkajoHjZU89GJ-2YcYTpPwoE6G7VbdVFVDFQISs-xPtF-l-f8WxOHRSD4HxH1wMod-pRMOypFrWIltWabHOZrB4f4SHM1NcNTrNEyJSwPZwFPvwXD-CSRkdlYPwzCtaN71pJJmn3wfL7XvNn-XHeebksfumM_Qb_4raQu54PWOwHkMYgU8DkL0B7-Rg4Gkhmr8LCcNUWgdLeEdlkcZEJWeR3Qn9eUMThJiulal2UMsy-Z3BOGb3M26vh0IW1BvEL8VcMsqFIupaOI77YGSTmEkC1DKxFHAVm3)



操作可参考这里：

[![asciicast](https://asciinema.org/a/313586.svg)](https://asciinema.org/a/313586)



`server` 的主要模块说明如下：

* `Admin Handlers` 处理来自 `admin` 端的请求；
* `Client Handlers` 处理来自用户的请求，其中对服务端到客户端的流，采取了阻塞队列的方式。可考虑使用 `Redis` 的 [BRPOPLPUSH](https://redis.io/commands/brpoplpush) 命令或持久化效果更好的消息队列；
* `Hub` 管理到用户的推送流和消息转发；
* `Utils` 为一些 `token` 生成/解码，`proto` 转换等方便函数或类；
* `DB` 存储用户注册登录等基本信息，配置了连接池。

其中客户端与服务端交互有：

* 注册用户信息(`register`)
* 登录(`login`)，并维持服务端到客户端的单向流，接收信息推送、保活等。推送信息可以是：变更的基本信息、系统广播信息，退出登录信息，在另一终端登录时原登录被踢出的通知信息等等，可根据消息类型灵活添加；用户在另一个终端登录时，会根据服务端配置的踢出策略（`kick_mode`, 1:踢出旧登录，2:保留旧登录，不能新登录）
* 修改基本用户信息(`update`)。此操作会返回成功，新的用户信息会通过服务端与客户端的单向流向客户端更新；
* 登出信息(`logout`)。此消息会在登录流中返回登出通知。

管理后台端与服务端的交互有：

* `Mock` 数据供客户端的各命令测试，由于做了签名校验，手动配置会麻烦，所以后端生成；
* 配置查看与设置（`config`），比如设置踢出模式、`token` 校验等级等；
* 服务端的 `metrics` 相关暴露（`inspect`），比如查询 `hub` 中活跃流数、连接池的数据情况等；
* 获取用户基本信息（`getuser`）接口，查看操作中数据是否存在或是否更新；
* 广播（`broadcast`），其实也是为了测试流推送情况。

### 数据流

1. 用户与 `client` 端交互时，采用 `命令 JSON包体` 的格式，`client` 与 `server` 交互采用`grpc/protobuf`;

2. 用户与 `admin` 端交互时，采用 `命令 子命令1/数据1 子命令2/数据2 ` 的格式，其中后面两个参数根据具体的命令可选。`admin` 与 `server` 交互采用 `grpc/protobuf`

由于 `Protobuf` 中定义的字段比较多，输入也不便，所以采用输入为 `JSON` 包体、`client` 将其转为 `protobuf` 格式再与 `server` 交互。另外由于做了一些签名校验，手动输入也会不大方便，所以`admin` 端提供了 `client` 端所有命令的即时 `mock` 数据,方便测试。



## 接口和协议



对 `CommonHeaderReq` 一些说明：

1. 每个请求都会带这个字段。`token` 是64位，对 `stamp` 是毫秒级时间戳。
2. 对注册请求，`token = md5(密码)+md5(验证码)`，实验中为方便测试，`mock` 数据生成的密码和验证码对应手机号中的中间四位和后四位。
3. 对非注册请求，`token = md5(src+key)+bin2hex(src)`, 其中 `src` 为 `systemType + timestamp` 组成的16位对其的字符串, `key` 为 `user_name`, 其中 `systemType` 定义见 `defs.hpp`，不超过8，`timestamp` 为毫秒级13位，填充16位没问题。即可以通过 `header` 直接校验 `token` 是否有效。在安全等级较高的情况下，由于 `timestamp` 和 `systemType` 等信息都存储在登录信息表中，也可以通过表中字段合成来校验。

对注册请求和登录请求，都会有一个签名校验，这个值为前面定义的所有字段的值转为字符串按序拼接成的长串的 `md5` 值。

对 `CommonHeaderResp`，`code` 可参考 `defs.hpp` 中的枚举定义，`message` 为枚举值对应的字符串。

登录成功后的返回为 `stream` 类型的 `LoginResp`，对 `LoginResp`, 可以有多种类型的消息推送，这里做了个抽象，`msg_type` 为可推送的消息类型（用户信息更新，接收广播信息，朋友圈更新，联系人更新，在另一台设备登录后被踢出等），`content` 字段比较灵活，可以是 `JSON` 包体。



```protobuf
syntax = "proto3";

package protocol.login.v1;

// Common Request Header
message CommonHeaderReq {
    string user_name = 1;
    string token = 2;
    uint64 stamp = 3;
}

// Common Response Header
message CommonHeaderResp {
    uint32 code = 1;
    string message = 2;
}

// DeviceInfo definitions
message DeviceInfo {
    // system_type can be: iOS, Android, Windows, MacOS, Linux, Web etc.
    uint32 system_type = 1;

    // device_type can be iPhone X, Huawei P30, Xiaomi 10 etc.
    uint32 device_type = 2;

    // channel_type can be App Store, Google Play, Mi Store etc.
    uint32 channel_type = 3;

    // device_name is the name set by the user, like: Ricky Gervais
    string device_name = 4;

    // device_id is the unique identifier of the device.
    string device_id = 5;
}

// Register Request
message RegisterReq {
    CommonHeaderReq header = 1;
    string phone_number = 2;

    DeviceInfo device_info = 3;
    string sign = 4;
}

// Register Response
message RegisterResp {
    CommonHeaderResp header = 1;
}

// Login Request
message LoginReq {
    CommonHeaderReq header = 1;
    uint32 login_type = 2;
    DeviceInfo device_info = 3;
    string sign = 4;
}

// Login Response
message LoginResp {
    message MessageItem {
        uint32 msg_type = 1;
        string content = 2;
    }

    CommonHeaderResp header = 1;
    repeated MessageItem messages = 2;
}

// Basic Info
message BasicInfoItem {
    string user_name = 1;
    string user_nick = 2;
    uint32 gender = 3;
    string avatar = 4;
    string signature = 5;
}

// Basic Info Update Request
message UpdateBasicInfoReq {
    CommonHeaderReq header = 1;
    BasicInfoItem info = 2;
}

// Basic Info Update Response
message UpdateBasicInfoResp {
    CommonHeaderResp header = 1;
    BasicInfoItem info = 2;
}

// Logout Request
message LogoutReq {
    CommonHeaderReq header = 1;
}


// Logout Response
message LogoutResp {
   CommonHeaderResp header = 1;
}

// Admin Operations
// Note: this exists mainly to facilitate testing.
//       no validation operations(i.e. header check) are performed
message AdminReq {
    CommonHeaderReq header = 1;
    // cmd can be: change config, send broadcast, mock data etc.
    string cmd = 2; 
    // f1 and f2 are placeholders. 
    // The values can be subcommands or data or even empty
    // They vary in line with the specific `cmd`s
    string f1 = 3;
    string f2 = 4;
}

message AdminResp {
    CommonHeaderResp header = 1;
    string result = 2;
 }


// Login API 
service LoginAPI {
    rpc Register(RegisterReq) returns (RegisterResp){}
    rpc Login(LoginReq) returns (stream LoginResp){}
    rpc UpdateBasicInfo(UpdateBasicInfoReq) returns (UpdateBasicInfoResp){}
    rpc Logout(LogoutReq) returns (LogoutResp){}
    rpc AdminOp(AdminReq) returns (AdminResp){}
}

```



## 库表设计

共四张表：

* `user_register_tab` 用户注册信息表，`append-only`;
* `user_basic_tab` 用户基本信息表
* `user_secure_tab` 用户安全信息表，原则上需要对 `passwd` 做 `bcrypt` 等加密，当前只做 `md5` 处理
* `user_login_tab` 用户登录信息表

用户新注册时，会同时写 `register`, `secure` 和 `basic` 三张表。原则上是一事务。但是依赖的三方库中貌似没找到事务支持。

用户登录时，会添加登录记录，登出或踢出时，会更新登出时间。

`register` 和 `login` 两张表中的 `ip` 字段操作中没有用到。



```sql
DROP TABLE IF EXISTS `user_register_tab`;
CREATE TABLE `user_register_tab` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_name` varchar(16) NOT NULL,
  `system_type` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `device_type` int(11) unsigned NOT NULL DEFAULT '0',
  `channel_type` int(11) unsigned NOT NULL DEFAULT '0',
  `device_name` varchar(16) NOT NULL DEFAULT '',
  `device_id` varchar(64) NOT NULL DEFAULT '',
  `ip` char(16) NOT NULL DEFAULT '',
  `ctime` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_uniq_name` (`user_name`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;

CREATE TABLE `user_basic_tab` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_name` varchar(16) NOT NULL,
  `user_nick` varchar(32) NOT NULL DEFAULT '',
  `gender` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `avatar` varchar(255) NOT NULL DEFAULT '',
  `signature` varchar(64) NOT NULL DEFAULT '',
  `ctime` int(11) NOT NULL DEFAULT '0',
  `mtime` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_uniq_name` (`user_name`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `user_secure_tab`;
CREATE TABLE `user_secure_tab` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_name` varchar(16) NOT NULL,
  `phone_number` char(11) NOT NULL COMMENT '手机号，只考虑86的',
  `passwd` char(32) NOT NULL COMMENT 'md5串，实际中用bcrypt',
  `status` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ctime` int(11) unsigned NOT NULL DEFAULT '0',
  `mtime` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_uniq_name` (`user_name`) USING BTREE,
  UNIQUE KEY `idx_uniq_phone` (`phone_number`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `user_login_tab`;
CREATE TABLE `user_login_tab` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_name` varchar(16) NOT NULL,
  `system_type` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `device_type` int(11) unsigned NOT NULL DEFAULT '0',
  `channel_type` int(11) unsigned NOT NULL DEFAULT '0',
  `device_name` varchar(16) NOT NULL DEFAULT '',
  `device_id` varchar(64) NOT NULL DEFAULT '',
  `ip` char(16) NOT NULL DEFAULT '',
  `login_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `logout_time` bigint(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_uniq_user_login` (`user_name`,`login_time`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;
```



## 三方依赖

除了`gRPC` 和 `protobuf`，还有以下依赖：

* [JSON](https://github.com/nlohmann/json) , 对结构体的序列化和反序列化处理非常方便

* [MD5](https://github.com/joyeecheung/md5)

* [loguru](https://github.com/emilk/loguru)，日志库

* [mysql modern cpp](https://github.com/daotrungkien/mysql-modern-cpp) `C++1x` 风格的 `MySQL` 包装。
  
在使用 `mysql modern cpp` 过程中，出现或调整了以下几个问题：

* 没有事务支持（对应注册用户时写三张表的操作）
* 添加了对 `connect_options` 的 `JSON` 序列化/反序列化处理（`conn_options_jsonify.hpp`）
* 头文件依赖调整
* `MySQL` 8.x 中移除了 `my_bool` 类型，这里做了修改
* 编译时 `std::move` 的调整
* `prepared_statement` 预处理时查询结果似乎有问题，查询还是用 `query` 稳妥

## 安全性

由于主要是实验性质，所以并没有依赖安全相关的重操作，但是在设计上，有这方面的考虑。

* 用户数据表分安全信息和基本信息，安全信息中包含用户密码和手机号，原则上，密码需采用破解难度高的加密库（如`bcrypt`）,同时这个表最好置于另一个访问权限高的数据库中，通过专门的 `RPC` 服务调用。本次操作中，只取了 `md5` 码，同基本信息表置于同一数据库中直接访问数据库得到。
* 用户注册后，会返回 `token`，在随后的操作中，都会带着 `token` 来操作。但是不同的操作对 `token` 的安全等级不尽相同，因此对同一 `token` 也可能有不同的等级的鉴定。理想的方式是加密了些关键参数（如时间戳、系统类型），在需要调整安全等级时，虽然数据库没存储具体值，但是可以根据相关字段反解。（见上面对 `CommonHeaderReq` 的说明）
* 用户输入校验，对注册和请求做签名校验（见上面说明）。对每个具体字段的长度，大小，格式等，这里没有做校验，后续可以参考 `Envoy` 的 [Proto-gen-validate](https://github.com/envoyproxy/protoc-gen-validate)
* 可考虑添加 `gRPC` 框架中的[健康检查](https://github.com/grpc/grpc/issues/13962)

## 代码组织



代码组织如下：

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
│       ├── v1
│       │   └── login.proto
│       └── v2
│           └── login.proto
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
│   │       └── polyfill
│   │           ├── datetime.h
│   │           ├── function_traits.h
│   │           └── optional.hpp
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
│   ├── tests                         # 测试相关
│   │   ├── bin2ascii_test.cc
│   │   ├── config_test.cc
│   │   ├── json_test.cc
│   │   ├── loguru_test.cc
│   │   ├── md5_test.cc
│   │   ├── mysql_test.cc
│   │   └── strings_test.cc
│   └── utils                         # 工具箱
│       ├── blocking_queue.hpp        #   阻塞队列
│       ├── conn_options_jsonify.cc   #   对三方mysql依赖的连接选项的 json 对象化包装
│       ├── conn_options_jsonify.hpp
│       ├── conn_pool.hpp             #   连接池
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



设计上将 `proto` 文件目录和服务逻辑分离，并没有按照 `bazel` 教程将 `proto` 文件和代码放在一起。

配置的时候，将 `proto` 文件通过 `protoc` 工具链来生成，将生成的代码文件一起上库，这样的操作主要是考虑到协议变更不频繁，对应语言的代码也一目了然，方便开发时参考。

## 编译环境

```
MacOS 10.15.3 (19D76)

clang：Apple clang version 11.0.0 (clang-1100.0.33.17)

Bazel: 2.2.0, 编译时需加选项 `--cxxopt='-std=c++17'`
```

## 操作

操作可以参考前面的录屏示例。`login-client` 和 `login-admin` 不指定地址时，取的是 `127.0.0.1:12345`，之后直接输入 `help` 查看所支持的操作。

注意：

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