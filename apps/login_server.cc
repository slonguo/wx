#include <string>
#include <memory>

#include <grpc++/grpc++.h>
#include <google/protobuf/util/json_util.h>

#include "apis/cpp/protocol/v1/login.grpc.pb.h"
#include "apps/3rd/loguru.hpp"
#include "apps/login/config.hpp"
#include "apps/login/dao.hpp"
#include "apps/login/defs.hpp"
#include "apps/login/admin_handlers.hpp"
#include "apps/login/stream.hpp"
#include "apps/login/hub.hpp"
#include "apps/login/session.hpp"
#include "apps/3rd/json.hpp"

using std::string;

using std::make_shared;
using std::unique_ptr;
using std::shared_ptr;

using nlohmann::json;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;


using protocol::login::v1::AdminReq;
using protocol::login::v1::AdminResp;
using protocol::login::v1::CommonHeaderResp;
using protocol::login::v1::LoginReq;
using protocol::login::v1::LoginResp;
using protocol::login::v1::LogoutReq;
using protocol::login::v1::LogoutResp;
using protocol::login::v1::RegisterReq;
using protocol::login::v1::RegisterResp;
using protocol::login::v1::UpdateBasicInfoReq;
using protocol::login::v1::UpdateBasicInfoResp;

using google::protobuf::util::MessageToJsonString;

using login::Config;
using login::Dao;
using login::Hub;
using login::KickMode;
using login::LoginSucceedResp;
using login::MsgItem;
using login::MsgType;
using login::RespCode;
using login::StreamItem;
using login::SvrConfig;
using login::UserBasicInfo;
using login::UserLoginInfo;
using login::UserSecureInfo;
using login::BasicSession;

using loguru::FileMode;
using loguru::Verbosity;

login::AdminHandlers adminHanlders;

class LoginServiceImpl final : public protocol::login::v1::LoginAPI::Service
{
public:
    LoginServiceImpl()
    {
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
        options.preserve_proto_field_names = true;
    }

    Status Register(ServerContext *ctx, const RegisterReq *req, RegisterResp *resp)
    {
        string jsonP;
        MessageToJsonString(*req, &jsonP, options);
        LOG_F(INFO, "register request:\n%s", jsonP.c_str());

        CommonHeaderResp header;

        if (!login::validRegisterReq(req))
        {
            LOG_F(INFO, "request invalid");
            SetCommonHeaderResp(header, RespCode::Invalid_Header_Request);
            resp->mutable_header()->CopyFrom(header);
            return Status::OK;
        }
        LOG_F(INFO, "req fields check ok");

        string userName(req->header().user_name());
        string phoneNumber(req->phone_number());
        UserSecureInfo usi;

        if (Dao::Instance().GetUserSecureInfo(userName, usi))
        {
            SetCommonHeaderResp(header, RespCode::User_Already_Registered);
            resp->mutable_header()->CopyFrom(header);
            return Status::OK;
        }

        LOG_F(INFO, "user name %s not registered, check phone %s", userName.c_str(), phoneNumber.c_str());
        if (Dao::Instance().GetUserSecureInfoByPhone(phoneNumber, usi))
        {
            SetCommonHeaderResp(header, RespCode::User_Already_Registered);
            resp->mutable_header()->CopyFrom(header);
            return Status::OK;
        }

        LOG_F(INFO, "phone number %s and user name %s not registered, start registering...", userName.c_str(), phoneNumber.c_str());
        if (!Dao::Instance().AddRegisterInfo(req))
        {
            LOG_F(INFO, "internal db op failed.request:\n%s", jsonP.c_str());
            SetCommonHeaderResp(header, RespCode::Internal_DB_Error);
            resp->mutable_header()->CopyFrom(header);
            return Status::OK;
        }

        LOG_F(INFO, "register ok for:%s", req->phone_number().c_str());

        SetCommonHeaderResp(header, RespCode::OK);
        resp->mutable_header()->CopyFrom(header);
        string token = BasicSession::Instance().genToken(userName, req->device_info().system_type());
        resp->set_token(token);

        return Status::OK;
    }

    Status Login(ServerContext *ctx, const LoginReq *req, ServerWriter<LoginResp> *writer)
    {
        string jsonP;
        MessageToJsonString(*req, &jsonP, options);
        LOG_F(INFO, "login request:%s\n", jsonP.c_str());

        auto deviceInfo = req->device_info();

        UserBasicInfo ubi;
        CommonHeaderResp header;
        SetCommonHeaderResp(header, RespCode::OK);


        // validLoginReq currently only does token/session check here to facilitate data mocking
        // skipped (username/phone,passwd) login
        // this function can be easily extended with the login_type differentiator in request
        LoginResp resp;
        if (!login::validLoginReq(req))
        {
            LOG_F(INFO, "request invalid");
            SetCommonHeaderResp(header, RespCode::Invalid_Header_Request);
            resp.mutable_header()->CopyFrom(header);

            writer->Write(resp);
            return Status::OK;
        }

        string userName = req->header().user_name();
        if (!Dao::Instance().GetUserBasicInfo(userName, ubi))
        {
            SetCommonHeaderResp(header, RespCode::User_Not_Found);
            resp.mutable_header()->CopyFrom(header);
            writer->Write(resp);
            return Status::OK;
        }

        // if user is already logged in
        // 1. check kick mode, if it's kick old then
        //      1. get his last login info
        //      2. update last logout time
        //      3. send a kickout message to previous stream
        //      4. drain old stream
        // 2. else return already logged in
        if (Hub::Instance().GetUser(userName))
        {

            auto kickMode = Config::Instance().GetKickMode();
            LOG_F(ERROR, "user %s already connected, try kick mode:%d", userName.c_str(), int(kickMode));
            if (kickMode == KickMode::KM_KickOld)
            {
                LOG_F(INFO, "kick-old mode: user %s already logged in, now do the kickout operation", userName.c_str());
                UserLoginInfo uli;
                // in normal cases this shouldn't happen
                if (!Dao::Instance().GetUserLastLoginInfo(userName, uli))
                {
                    LOG_F(INFO, "oops, get user last login info failed:%s", userName.c_str());
                    SetCommonHeaderResp(header, RespCode::Internal_DB_Error);
                    resp.mutable_header()->CopyFrom(header);
                    writer->Write(resp);
                    return Status::OK;
                }

                Dao::Instance().UpdateLogoutInfo(userName);

                string deviceS;
                MessageToJsonString(deviceInfo, &deviceS, options);

                LOG_F(INFO, "kickout resp msg for %s:\n%s", userName.c_str(), deviceS.c_str());
                shared_ptr<MsgItem> pMsg = make_shared<MsgItem>(MsgType::MT_KickedOut, deviceS);

                Hub::Instance().AddMsg(userName, pMsg);
            }
            else
            {
                LOG_F(INFO, "kick-new mode. under this mode old user persists. new login won't succeed for %s", userName.c_str());
                SetCommonHeaderResp(header, RespCode::User_Already_Logged_In);
                resp.mutable_header()->CopyFrom(header);
                writer->Write(resp);
                return Status::OK;
            }
        }

        // make a new stream
        shared_ptr<StreamItem> userStream = make_shared<StreamItem>(userName);

        Hub::Instance().AddUser(userName, userStream);
        Dao::Instance().AddLoginInfo(req);

        // for new stream, send user basic info
        shared_ptr<MsgItem> pBasic = make_shared<MsgItem>(MsgType::MT_LoginSucceed, json(ubi).dump(4));
        Hub::Instance().AddMsg(userName, pBasic);

        while (1)
        {
            // optimize: batch send
            shared_ptr<MsgItem> pItem = userStream->GetMsgItem();
            if (!login::IsValidMsgType(pItem->type))
            {
                LOG_F(INFO, "invalid msg type:%d, msg:%s", pItem->type, pItem->content.c_str());
                continue;
            }

            LoginResp resp;
            LoginResp::MessageItem mi;
            mi.set_msg_type(pItem->type);
            mi.set_content(pItem->content);

            bool isValidLogin = login::IsValidLoginType(pItem->type);
            if(!isValidLogin)
            {
                SetCommonHeaderResp(header, RespCode::User_Logout_Or_Kicked_Out);
            }
            resp.mutable_header()->CopyFrom(header);
            resp.mutable_messages()->Add(std::move(mi));
            writer->Write(resp);

            // user logout/kicked ...
            if (!isValidLogin)
            {
                LOG_F(INFO, "user %s logged/kicked out. type:%d, msg:%s", userName.c_str(), pItem->type, pItem->content.c_str());
                LOG_F(INFO, "draining stream for %s", userName.c_str());

                // update logout info to db
                Dao::Instance().UpdateLogoutInfo(userName);

                return Status::OK;
            }

            // seems redundant
            // in normal cases this won't be executed
            if (userStream.use_count() == 1)
            {
                // something abnormal
                LOG_F(ERROR, "oops!! anomaly captured, user:%s stream use_count == 1", userName.c_str());
                return Status(StatusCode::INTERNAL, grpc::string("old user[" + userName + "] stream not destructed"));
            }
        }

        return Status::OK;
    }

    Status UpdateBasicInfo(ServerContext *ctx, const UpdateBasicInfoReq *req, UpdateBasicInfoResp *resp)
    {
        string jsonP;
        MessageToJsonString(*req, &jsonP, options);
        LOG_F(INFO, "update request:%s\n", jsonP.c_str());

        string dest;
        CommonHeaderResp header;
        SetCommonHeaderResp(header, RespCode::OK);

        if (!login::validHeaderReq(req->header(), dest))
        {
            LOG_F(INFO, "request invalid");
            SetCommonHeaderResp(header, RespCode::Invalid_Header_Request);
            resp->mutable_header()->CopyFrom(header);
            return Status::OK;
        }

        string userName(req->header().user_name());
        Dao::Instance().UpdateUserBasicInfo(userName, req->info());
        string infoS;
        MessageToJsonString(req->info(), &infoS, options);
        shared_ptr<MsgItem> pItem = make_shared<MsgItem>(MsgType::MT_UserInfo, infoS);
        Hub::Instance().AddMsg(userName, pItem);

        return Status::OK;
    }

    Status Logout(ServerContext *ctx, const LogoutReq *req, LogoutResp *resp)
    {
        string jsonP;
        MessageToJsonString(*req, &jsonP, options);
        LOG_F(INFO, "logout request:%s\n", jsonP.c_str());

        string dest;
        CommonHeaderResp header;
        SetCommonHeaderResp(header, RespCode::OK);


        if (!login::validHeaderReq(req->header(), dest))
        {
            LOG_F(INFO, "request invalid");
            SetCommonHeaderResp(header, RespCode::Invalid_Header_Request);
            resp->mutable_header()->CopyFrom(header);
            return Status::OK;
        }

        string userName = req->header().user_name();
        shared_ptr<MsgItem> pItem = make_shared<MsgItem>(MsgType::MT_Logout, "user logout request");
        Hub::Instance().AddMsg(userName, pItem);
        Hub::Instance().DelUser(userName);

        return Status::OK;
    }

    Status AdminOp(ServerContext *ctx, const AdminReq *req, AdminResp *resp)
    {
        string jsonP;
        MessageToJsonString(*req, &jsonP, options);
        LOG_F(INFO, "admin request:\n%s", jsonP.c_str());
        CommonHeaderResp header;
        SetCommonHeaderResp(header, RespCode::OK);

        string msg = req->cmd();
        string f1 = req->f1();
        string f2 = req->f2();
        adminHanlders.HandleAdminReq(msg, f1, f2, resp);
        return Status::OK;
    }

private:
    google::protobuf::util::JsonPrintOptions options;
};

void RunServer(SvrConfig sc)
{
    string serverAddr(sc.endpoint);
    LoginServiceImpl loginService;

    ServerBuilder builder;

    // TODO(slonguo): use secure credentials
    builder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
    builder.RegisterService(&loginService);

    unique_ptr<Server> server(builder.BuildAndStart());
    LOG_F(INFO, "server starts at:%s", sc.endpoint.c_str());
    server->Wait();
}

string getBinName(string str, char c = '/')
{
    size_t pos = str.rfind(c);
    if (pos == string::npos)
        return str;
    return str.substr(pos + 1, str.size() - pos);
}

int main(int argc, char **argv)
{

    loguru::init(argc, argv);

    // get and parse config file.
    // use ${bin}.json as the default config if no input is specified.
    string defaultConfigFile = getBinName(argv[0]) + ".json";
    LOG_IF_F(INFO, argc == 1, "no input config, using %s as default", defaultConfigFile.c_str());
    string configFile = (argc > 1) ? argv[1] : defaultConfigFile;
    if (!Config::Instance().Init(configFile))
    {
        LOG_F(FATAL, "error in parsing config file:%s", configFile.c_str());
        return -1;
    }

    // log settings
    auto logConf = Config::Instance().GetLogConfig();
    loguru::add_file(logConf.file_name.c_str(), FileMode(logConf.file_mode), Verbosity(logConf.level));
    loguru::g_stderr_verbosity = logConf.verbosity;

    json j = Config::Instance().GetConfig();
    LOG_F(INFO, "server config info(user/password obscured):\n\n%s\n", j.dump(4).c_str());

    // init db
    auto dbConf = Config::Instance().GetDBConfig();
    LOG_IF_F(FATAL, !Dao::Instance().Init(dbConf), "db init failed");

    // start server
    auto svrConf = Config::Instance().GetSvrConfig();
    RunServer(svrConf);

    return 0;
}
