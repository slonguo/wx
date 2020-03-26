#ifndef LOGIN_HANDLERS_HPP
#define LOGIN_HANDLERS_HPP

#include <string>
#include <unordered_map>

#include "proto_helper.hpp"
#include "apis/cpp/protocol/v1/login.pb.h"

namespace login
{
using std::string;
using std::unordered_map;

using protocol::login::v1::AdminResp;

class AdminHandlers
{
public:
    explicit AdminHandlers() { Init(); };

    void Init();
    bool HandleAdminReq(string, string, string, AdminResp *);

    AdminHandlers(const AdminHandlers &) = delete;
    AdminHandlers &operator=(const AdminHandlers &) = delete;

private:
    void setConfig(string, string, AdminResp *);
    void mockData(string, string, AdminResp *);
    void inspectServer(string, string, AdminResp *);
    void broadcast(string, string, AdminResp *);
    void getUser(string, string, AdminResp *);

private:
    using func = void (AdminHandlers::*)(string, string, AdminResp *);

    unordered_map<string, func> cmdHandlers;
    unordered_map<string, login::mockFunc> mockHandlers;
};
} // namespace login

#endif