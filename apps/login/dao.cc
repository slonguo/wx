#include <iostream>
#include <string>
#include "dao.hpp"
#include "apps/3rd/sql/mysqlplus.h"
#include "apps/utils/time.hpp"
#include "apps/3rd/loguru.hpp"
#include "apps/3rd/json.hpp"
#include "apps/login/defs.hpp"

namespace login
{

using daotk::mysql::mysql_exception;
using daotk::mysql::mysqlpp_exception;

using login::UserStatus;

bool Dao::Init(DBConfig dc)
{
    return cp_.Init(dc);
}

bool Dao::AddRegisterInfo(const RegisterReq *req)
{
    LOG_F(INFO, "start add register info");
    string passwd(req->header().token().substr(0, 32));
    shared_ptr<connection> conn = cp_.get();
    uint64_t nowT = utils::NowSec();

    auto header = req->header();
    auto deviceInfo = req->device_info();
    string userName = header.user_name();
    string phoneNumber = req->phone_number();

    // TODO(slonguo): transaction needed
    // result check omitted

    // begin transaction
    prepared_stmt s1(
        *conn.get(),
        "insert into user_secure_tab set user_name = ?, phone_number = ?, passwd = ?, status = ?, ctime = ?, mtime = ?");

    s1.bind_param(userName, phoneNumber, passwd, int(US_Init), nowT, nowT);
    s1.execute();
    LOG_F(INFO, "secure info added");

    prepared_stmt s2(
        *conn.get(),
        "insert into user_basic_tab set user_name = ?, ctime=?, mtime=?");
    s2.bind_param(userName, nowT, nowT);
    s2.execute();
    LOG_F(INFO, "basic info added");

    prepared_stmt s3(
        *conn.get(),
        "insert into user_register_tab set user_name = ?, system_type = ?, device_type = ?, channel_type = ?, device_name = ?, device_id = ?, ctime = ?");
    s3.bind_param(
        userName,
        deviceInfo.system_type(),
        deviceInfo.device_type(),
        deviceInfo.channel_type(),
        deviceInfo.device_name(),
        deviceInfo.device_id(),
        nowT);
    s3.execute();

    LOG_F(INFO, "register info added");

    // end transaction
    cp_.release(conn);
    return true;
}

bool Dao::GetUserBasicInfo(string userName, UserBasicInfo &ubi)
{
    shared_ptr<connection> conn = cp_.get();
    auto res = conn->query("select user_name, user_nick, gender, avatar, signature from user_basic_tab where user_name = \"%s\" limit 1", userName.c_str());
    if (!res.is_empty())
    {
        res.fetch(ubi.userName, ubi.userNick, ubi.gender, ubi.avatar, ubi.signature);
    }

    cp_.release(conn);
    return !res.is_empty();
}

bool Dao::GetUserSecureInfo(string userName, UserSecureInfo &usi)
{
    shared_ptr<connection> conn = cp_.get();
    auto res = conn->query("select user_name, select phone_number, passwd, status from user_secure_tab where user_name = \"%s\" limit 1", userName.c_str());
    if (!res.is_empty())
    {
        res.fetch(usi.userName, usi.phoneNumber, usi.passwd, usi.status);
    }

    cp_.release(conn);
    return !res.is_empty();
}

bool Dao::GetUserSecureInfoByPhone(string phoneNumber, UserSecureInfo usi)
{
    LOG_F(INFO, "begin GetUserSecureInfoByPhone. phone:%s", phoneNumber.c_str());
    shared_ptr<connection> conn = cp_.get();
    auto res = conn->query("select user_name, phone_number, passwd, status from user_secure_tab where phone_number = \"%s\" limit 1", phoneNumber.c_str());
    if (!res.is_empty())
    {
        res.fetch(usi.userName, usi.phoneNumber, usi.passwd, usi.status);
    }

    cp_.release(conn);
    return !res.is_empty();
}

bool Dao::GetUserLastLoginInfo(string userName, UserLoginInfo &uli)
{
    shared_ptr<connection> conn = cp_.get();
    auto res = conn->query(
        "select system_type, device_type, channel_type, device_name, device_id, ip, login_time, logout_time from user_login_tab where user_name = \"%s\" order by login_time desc limit 1",
        userName.c_str());
    if (!res.is_empty())
    {
        res.fetch(uli.systemType, uli.deviceType, uli.channelType, uli.deviceName, uli.deviceID, uli.IP, uli.loginTime, uli.logoutTime);
    }

    cp_.release(conn);
    return !res.is_empty();
}

bool Dao::UpdateUserBasicInfo(string userName, const BasicInfoItem &ubi)
{
    shared_ptr<connection> conn = cp_.get();
    uint64_t nowT = utils::NowSec();

    prepared_stmt s1(
        *conn.get(),
        "update user_basic_tab set user_nick = ?, gender = ?, avatar = ?, signature = ?, mtime = ? where user_name = ? limit 1");

    s1.bind_param(ubi.user_nick(), ubi.gender(), ubi.avatar(), ubi.signature(), nowT, userName);
    s1.execute();
    cp_.release(conn);

    return true;
}

bool Dao::AddLoginInfo(const LoginReq *req)
{
    shared_ptr<connection> conn = cp_.get();
    uint64_t nowT = utils::NowSec();

    auto header = req->header();
    auto deviceInfo = req->device_info();
    string userName = header.user_name();

    prepared_stmt stmt(
        *conn.get(),
        "insert into user_login_tab set user_name = ?, system_type = ?, device_type = ?, channel_type = ?, device_name = ?, device_id = ?, login_time = ?");

    stmt.bind_param(
        userName,
        deviceInfo.system_type(),
        deviceInfo.device_type(),
        deviceInfo.channel_type(),
        deviceInfo.device_name(),
        deviceInfo.device_id(),
        nowT);
    stmt.execute();

    cp_.release(conn);
    return true;
}
bool Dao::UpdateLogoutInfo(string userName)
{
    shared_ptr<connection> conn = cp_.get();
    uint64_t nowT = utils::NowSec();
    prepared_stmt stmt(
        *conn.get(),
        "update user_login_tab set logout_time = ? where user_name = ? order by login_time desc limit 1");
    stmt.bind_param(nowT, userName);
    stmt.execute();

    cp_.release(conn);
    return true;
}
} // namespace login