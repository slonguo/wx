#ifndef LOGIN_CONFIG_HPP
#define LOGIN_CONFIG_HPP

#include <mutex>
#include <shared_mutex>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "apps/3rd/loguru.hpp"
#include "apps/3rd/json.hpp"
#include "apps/3rd/sql/mysqlplus.h"
#include "apps/utils/conn_options_jsonify.hpp"



namespace login
{

using std::ifstream;
using std::shared_lock;
using std::shared_mutex;
using std::string;
using std::unique_lock;
using std::string;

using daotk::mysql::connect_options;
using nlohmann::json;

using utils::DBConfig;

struct SvrConfig
{
    string endpoint;
    // string healthKey;
};

void to_json(json &j, const SvrConfig &sc);
void from_json(const json &j, SvrConfig &sc);

enum KickMode
{
    KM_Init = 0,
    KM_KickOld,
    KM_KickNew,
    KM_CNT,
};

struct AdminConfig
{
    KickMode kick_mode;
};

void to_json(json &j, const AdminConfig &ac);
void from_json(const json &j, AdminConfig &ac);

// see loguru.hpp for enumeration definitons
struct LogConfig
{
    string file_name;
    int file_mode; // enum FileMode { Truncate, Append };
    int level;     // Info(0) Warn(-1) Error(-2) Fatal(-3)
    int verbosity; // verbosity level: 1 -> 9
};

void to_json(json &j, const LogConfig &lc);
void from_json(const json &j, LogConfig &lc);

struct LoginConfig
{
    SvrConfig svr;
    AdminConfig admin;
    DBConfig db;
    LogConfig log;
};

void to_json(json &j, const LoginConfig &lc);
void from_json(const json &j, LoginConfig &lc);



class Config
{
public:
    static Config &Instance()
    {
        static Config c;
        return c;
    }

    Config(const Config &) = delete;
    Config &operator=(Config &) = delete;

    bool Init(string configFile)
    {
        if (configFile.empty())
            return false;
        ifstream s(configFile);
        if (!s.good())
            return false;

        try
        {
            json j;
            s >> j;
            lc_ = j.get<LoginConfig>();
        }
        catch (...)
        {
            LOG_F(ERROR, "JSON deserialization error for file:%s", configFile.c_str());
            return false;
        }
        return true;
    }

    const LoginConfig &GetConfig() { return lc_; }
    const LogConfig &GetLogConfig() { return lc_.log; }
    const DBConfig &GetDBConfig() { return lc_.db; }
    const SvrConfig &GetSvrConfig() { return lc_.svr; }

    const AdminConfig &GetAdminConfig()
    {
        shared_lock<shared_mutex> lock(mutex_);
        return lc_.admin;
    }

    KickMode GetKickMode()
    {
        shared_lock<shared_mutex> lock(mutex_);
        return lc_.admin.kick_mode;
    }

    bool SetKickMode(KickMode km)
    {
        if (km > KM_Init && km < KM_CNT)
        {
            unique_lock<shared_mutex> lock(mutex_);
            lc_.admin.kick_mode = km;
            return true;
        }
        return false;
    }

private:
    Config() {}

private:
    login::LoginConfig lc_;
    mutable shared_mutex mutex_;
};


}


#endif