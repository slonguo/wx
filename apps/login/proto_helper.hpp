#ifndef LOGIN_PROTO_HELPER_HPP
#define LOGIN_PROTO_HELPER_HPP

#include <string>
#include "apps/3rd/json.hpp"

#include "apis/cpp/protocol/v1/login.pb.h"

namespace login
{
using std::string;

using nlohmann::json;
using protocol::login::v1::BasicInfoItem;
using protocol::login::v1::CommonHeaderReq;
using protocol::login::v1::CommonHeaderResp;
using protocol::login::v1::DeviceInfo;
using protocol::login::v1::LoginReq;
using protocol::login::v1::LoginResp;
using protocol::login::v1::LogoutReq;
using protocol::login::v1::LogoutResp;
using protocol::login::v1::RegisterReq;
using protocol::login::v1::RegisterResp;
using protocol::login::v1::UpdateBasicInfoReq;
using protocol::login::v1::UpdateBasicInfoResp;

using mockFunc = string (*)(string);

json mockHeader(string userName, string &headerS);
json mockRegisterHeader(string userName, string &phone, string &headerS);
json mockDevice(string &);
string mockLoginData(string userName);
string mockRegisterData(string userName);
string mockLogoutData(string userName);
string mockUpdateData(string userName);

string toString(const DeviceInfo &di);
string toString(const CommonHeaderReq &header);
bool validHeaderReq(const CommonHeaderReq &req, string &);
bool validRegisterReq(const RegisterReq *req);
bool validLoginReq(const LoginReq *req);

} // namespace login

#endif