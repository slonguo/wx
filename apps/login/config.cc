#include "config.hpp"

namespace login
{
void to_json(json &j, const SvrConfig &sc)
{
    j = json{
        {"endpoint", sc.endpoint},
        // {"health_key", sc.healthKey},
    };
}

void from_json(const json &j, SvrConfig &sc)
{
    j.at("endpoint").get_to(sc.endpoint);
    // j.at("health_key").get_to(sc.healthKey);
}

void to_json(json &j, const AdminConfig &ac)
{
    j = json{
        {"kick_mode", ac.kick_mode},
    };
}

void from_json(const json &j, AdminConfig &ac)
{
    j.at("kick_mode").get_to(ac.kick_mode);
}

void to_json(json &j, const LogConfig &lc)
{
    j = json{
        {"file_name", lc.file_name},
        {"file_mode", lc.file_mode},
        {"level", lc.level},
        {"verbosity", lc.verbosity},
    };
}

void from_json(const json &j, LogConfig &lc)
{
    j.at("file_name").get_to(lc.file_name);
    j.at("file_mode").get_to(lc.file_mode);
    j.at("level").get_to(lc.level);
    j.at("verbosity").get_to(lc.verbosity);
}

void to_json(json &j, const LoginConfig &lc)
{
    j = json{
        {"server", lc.svr},
        {"admin", lc.admin},
        {"db", lc.db},
        {"log", lc.log},
    };
}

void from_json(const json &j, LoginConfig &lc)
{
    j.at("server").get_to(lc.svr);
    j.at("admin").get_to(lc.admin);
    j.at("db").get_to(lc.db);
    j.at("log").get_to(lc.log);
}

} 