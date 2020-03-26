#include <memory>
#include "admin_handlers.hpp"
#include "config.hpp"

#include "apps/3rd/json.hpp"
#include "apps/utils/strings.hpp"
#include "hub.hpp"
#include "defs.hpp"
#include "dao.hpp"

namespace login
{
using std::make_shared;
using std::shared_ptr;

using nlohmann::json;

void AdminHandlers::Init()
{
    cmdHandlers = {
        {"config", &AdminHandlers::setConfig},
        {"mock", &AdminHandlers::mockData},
        {"inspect", &AdminHandlers::inspectServer},
        {"broadcast", &AdminHandlers::broadcast},
        {"getuser", &AdminHandlers::getUser},
    };

    mockHandlers = {
        {"register", &login::mockRegisterData},
        {"login", &login::mockLoginData},
        {"logout", &login::mockLogoutData},
        {"update", &login::mockUpdateData},
    };
}

bool AdminHandlers::HandleAdminReq(string cmd, string s1, string s2, AdminResp *resp)
{
    SetAdminResp(resp, RespCode::OK);

    auto it = cmdHandlers.find(cmd);
    if (it != cmdHandlers.end())
    {
        (this->*(it->second))(s1, s2, resp);
        return true;
    }

    SetAdminResp(resp, RespCode::Invalid_Admin_Command);
    return false;
}

void AdminHandlers::mockData(string subOp, string userName, AdminResp *resp)
{
    auto it = mockHandlers.find(subOp);
    if (it == mockHandlers.end())
        return;

    if (!utils::isAlphaNum(userName))
    {
        SetAdminResp(resp, RespCode::Invalid_Subcommand_Or_Data);
        return;
    }
    auto result = (*it->second)(userName);
    resp->set_result(result);
}

void AdminHandlers::setConfig(string s1, string s2, AdminResp *resp)
{
    if (s1 == "set")
    {
        try
        {
            auto j = json::parse(s2);
            if (j.find("kick_mode") != j.end())
            {
                auto km = j["kick_mode"].get<int>();
                if (km > login::KM_Init && km < login::KM_CNT)
                {
                    Config::Instance().SetKickMode(login::KickMode(km));
                    return;
                }
            }
        }
        catch (...)
        {
            LOG_F(ERROR, "bad input:[%s, %s]", s1.c_str(), s2.c_str());
        }
    }
    else if (s1 == "show")
    {
        json j = Config::Instance().GetConfig();
        resp->set_result(j.dump(4));
        return;
    }

    SetAdminResp(resp, RespCode::Invalid_Config);
}

void AdminHandlers::inspectServer(string s1, string s2, AdminResp *resp)
{
    // stats about: connection pool, hub, queue etc.
    // s1 and s2 are currently not used
    auto hubStats = Hub::Instance().GetHubStats();
    auto daoStats = Dao::Instance().GetDaoStats();
    json j = {
        {"hub", {{"stream_count", hubStats.streamCount}}},
        {"db", {{"pool",{{"pool_size", daoStats.poolSize}, {"borrowed_size", daoStats.borrowedSize}}}}},
    };
    resp->set_result(j.dump(4));
}

void AdminHandlers::broadcast(string s1, string s2, AdminResp *resp)
{
    // broadcast is used to test grpc stream
    // s2 is only a placeholder
    shared_ptr<MsgItem> pItem = make_shared<MsgItem>(MsgType::MT_Broadcast, s1);
    Hub::Instance().Broadcast(pItem);
}

void AdminHandlers::getUser(string s1, string s2, AdminResp *resp)
{
    UserBasicInfo ubi;
    if(Dao::Instance().GetUserBasicInfo(s1, ubi)){
        resp->set_result(json(ubi).dump(4));
    }else{
        resp->set_result("no such user");
    }
}
} // namespace login