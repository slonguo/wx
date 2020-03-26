
#include "conn_options_jsonify.hpp"
#include "apps/3rd/loguru.hpp"

// customize JSON serializer/deserializer for connect_options to facilitate config parsing
namespace daotk {
namespace mysql {

void to_json(json& j, const connect_options& co)
{
    j = json
    {
        {"server", co.server},

        // for security's sake, username and passwd are not exposed
        // {"username", co.username},
        // {"password", co.password},
        {"username", "***hidden***"},
        {"password", "***hidden***"},

        {"dbname", co.dbname},
        {"timeout", co.timeout},
        {"autoreconnect", co.autoreconnect},
        {"init_command", co.init_command},
        {"charset", co.charset},
        {"port", co.port},
        {"client_flag", co.client_flag}
    };
}

void from_json(const json& j, connect_options& co)
{
    static const char * keyNotFoundFmt = "key:[%20s] not found in db conn config";
    // use try ... catch .. here to allow optional connection options.
    try{j.at("server").get_to(co.server);}catch(...){LOG_F(INFO, keyNotFoundFmt, "server");}
    try{j.at("username").get_to(co.username);}catch(...){LOG_F(INFO, keyNotFoundFmt, "username");}
    try{j.at("password").get_to(co.password);}catch(...){LOG_F(INFO, keyNotFoundFmt, "password");}
    try{j.at("dbname").get_to(co.dbname);}catch(...){LOG_F(INFO, keyNotFoundFmt, "dbname");}
    try{j.at("timeout").get_to(co.timeout);}catch(...){LOG_F(INFO, keyNotFoundFmt, "timeout");}
    try{j.at("autoreconnect").get_to(co.autoreconnect);}catch(...){LOG_F(INFO, keyNotFoundFmt, "autoreconnect");}
    try{j.at("init_command").get_to(co.init_command);}catch(...){LOG_F(INFO, keyNotFoundFmt, "init_command");}
    try{j.at("charset").get_to(co.charset);}catch(...){LOG_F(INFO, keyNotFoundFmt, "charset");}
    try{j.at("port").get_to(co.port);}catch(...){LOG_F(INFO, keyNotFoundFmt, "port");}
    try{j.at("client_flag").get_to(co.client_flag);}catch(...){LOG_F(INFO, keyNotFoundFmt, "client_flag");}
}

} // mysql
} // daotk

namespace utils{

void to_json(json &j, const DBConfig &dc)
{
    j = json
    {
        {"conn", dc.conn},
        {"pool_size", dc.pool_size}
    };
}

void from_json(const json &j, DBConfig &dc)
{
    j.at("conn").get_to(dc.conn);
    j.at("pool_size").get_to(dc.pool_size);
}

}