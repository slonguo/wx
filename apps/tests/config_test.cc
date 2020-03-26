#include <string>
#include <iostream>

#include "apps/login/config.hpp"
#include "apps/3rd/loguru.hpp"
#include "apps/3rd/json.hpp"

using std::string;
using nlohmann::json;
using loguru::FileMode;
using loguru::Verbosity;
using login::Config;

int main(int argc, char* argv[])
{
    loguru::init(argc, argv);
    LOG_F(INFO, "start confit test");
    string configFile = (argc > 1) ? argv[1] : "server.json";
    if(!Config::Instance().Init(configFile))
    {
        LOG_F(FATAL, "error in parsing config file:%s", configFile.c_str());
        return -1;
    }

    auto lc = Config::Instance().GetLogConfig();
    json j = Config::Instance().GetConfig();
    loguru::add_file(lc.file_name.c_str(), FileMode(lc.file_mode), Verbosity(lc.level));
    loguru::g_stderr_verbosity = lc.verbosity;
    LOG_F(INFO, "server config info:\n%s\n", j.dump(4).c_str());

    LOG_F(INFO, "config test OK");
    return 0;
}