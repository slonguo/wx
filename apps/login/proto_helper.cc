#include "proto_helper.hpp"

#include "apps/utils/time.hpp"
#include "apps/utils/strings.hpp"
#include "apps/3rd/md5.hpp"
#include "defs.hpp"

#include "session.hpp"

namespace login
{

using login::BasicSession;

auto ts = [](uint64_t v) -> string { return std::to_string(v); };

json mockHeader(string userName, string &src)
{
    uint64_t stamp = utils::NowSec();
    int sysType = utils::randInt(login::ST_Unknown + 1, login::ST_Count);
    string token = BasicSession::Instance().genToken(userName, sysType, stamp);

    json j = {
        {"user_name", userName},
        {"token", token},
        {"stamp", stamp},
    };

    src = userName + token + ts(stamp);
    return j;
}

json mockRegisterHeader(string userName, string &phone, string &headerS)
{
    uint64_t stamp = utils::NowSec();
    string s1 = utils::randGenNumStr(4);
    string s2 = utils::randGenNumStr(4);
    phone = "155" + s1 + s2;
    string token = joyee::md5(s1) + joyee::md5(s2);
    headerS = userName + token + ts(stamp);

    json j = {
        {"user_name", userName},
        {"token", token},
        {"stamp", stamp},
    };
    return j;
}

json mockDevice(string &src)
{
    int sysType = utils::randInt(login::ST_Unknown + 1, login::ST_Count);
    int devType = utils::randInt(login::DT_Unknown + 1000, login::DT_Count);
    int chanType = utils::randInt(login::CT_Unknown + 1, login::CT_Count);
    string devName = utils::randGenAlphaLowerCase(6);
    string devID = utils::randGenAlphaLowerCase(64);
    json j = {
        {"system_type", sysType},
        {"device_type", devType},
        {"channel_type", chanType},
        {"device_name", devName},
        {"device_id", devID},
    };
    src = ts(sysType) + ts(devType) + ts(chanType) + devName + devID;
    return j;
}

string mockRegisterData(string userName)
{

    string headerS, deviceS, phoneNumber;
    json headerJ = mockRegisterHeader(userName, phoneNumber, headerS);
    json deviceJ = mockDevice(deviceS);

    string sign = joyee::md5(headerS + phoneNumber + deviceS);
    json j = {
        {"header", headerJ},
        {"phone_number", phoneNumber},
        {"device_info", deviceJ},
        {"sign", sign},
    };

    return j.dump();
}

string mockLoginData(string userName)
{
    string headerS, deviceS;
    json headerJ = mockHeader(userName, headerS);
    json deviceJ = mockDevice(deviceS);
    int loginType = utils::randInt(login::LT_Unknown + 1, login::LT_Count);
    string sign = joyee::md5(headerS + ts(loginType) + deviceS);
    json j = {
        {"header", headerJ},
        {"login_type", loginType},
        {"device_info", deviceJ},
        {"sign", sign},
    };
    return j.dump();
}

string mockLogoutData(string userName)
{
    string headerS;
    json headerJ = mockHeader(userName, headerS);
    json j = {
        {"header", headerJ},
    };
    return j.dump();
}

string mockUpdateData(string userName)
{
    string headerS;
    json headerJ = mockHeader(userName, headerS);
    json basicJ = {
        {"user_name", userName},
        {"avatar", utils::randGenAlphaNum(20)},
        {"user_nick", utils::randGenAlphaLowerCase(10)},
        {"gender", utils::randInt(1, 2)},
        {"signature", utils::randGenAlphaNum(30)},
    };
    json j = {
        {"header", headerJ},
        {"info", basicJ},
    };
    return j.dump();
}

string toString(const DeviceInfo &di)
{
    return ts(di.system_type()) + ts(di.device_type()) + ts(di.channel_type()) + di.device_name() + di.device_id();
}

string toString(const CommonHeaderReq &header)
{
    return header.user_name() + header.token() + ts(header.stamp());
}

bool validHeaderReq(const CommonHeaderReq &req, string &result)
{
    return BasicSession::Instance().decodeSession(req.token(), req.user_name(), result);
}

bool validRegisterReq(const RegisterReq *req)
{
    return joyee::md5(toString(req->header()) + req->phone_number() + toString(req->device_info())) == req->sign();
}

bool validLoginReq(const LoginReq *req)
{
    // validLoginReq currently only does token/session check here to facilitate data mocking
    // skipped (username/phone,passwd) login
    // this function can be easily extended with the login_type differentiator in request
    string dest;
    if (!validHeaderReq(req->header(), dest))
    {
        return false;
    }
    return joyee::md5(toString(req->header()) + ts(req->login_type()) + toString(req->device_info())) == req->sign();
}

} // namespace login