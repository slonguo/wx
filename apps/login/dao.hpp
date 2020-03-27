
#ifndef LOGIN_DAO_HPP
#define LOGIN_DAO_HPP

#include <mutex>
#include <deque>
#include <set>

#include "defs.hpp"
#include "config.hpp"
#include "apps/utils/conn_pool.hpp"

#include "apis/cpp/protocol/v1/login.pb.h"

namespace login
{

using std::deque;
using std::mutex;
using std::set;
using std::shared_ptr;

using daotk::mysql::connect_options;
using daotk::mysql::connection;
using daotk::mysql::prepared_stmt;
using login::UserBasicInfo;
using login::UserLoginInfo;
using login::UserSecureInfo;
using protocol::login::v1::BasicInfoItem;
using protocol::login::v1::LoginReq;
using protocol::login::v1::RegisterReq;

using utils::ConnPool;
using utils::DBConfig;
using utils::PoolStats;

class Dao
{
public:
    static Dao &Instance()
    {
        static Dao d;
        return d;
    }

    Dao(const Dao &) = delete;
    Dao &operator=(Dao &) = delete;

    bool Init(DBConfig);

    PoolStats GetDaoStats() { return cp_.getStats(); }

    bool AddRegisterInfo(const RegisterReq *req);
    bool GetUserBasicInfo(string userName, UserBasicInfo &ubi);
    bool GetUserSecureInfo(string userName, UserSecureInfo &usi);
    bool GetUserSecureInfoByPhone(string userName, UserSecureInfo &usi);
    bool GetUserLastLoginInfo(string userName, UserLoginInfo &uli);
    bool UpdateUserBasicInfo(string userName, const BasicInfoItem &ubi);
    bool AddLoginInfo(const LoginReq *req);
    bool UpdateLogoutInfo(string userName);

private:
    Dao() {}

private:
    ConnPool cp_;
};
} // namespace login

#endif