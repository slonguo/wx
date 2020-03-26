#include <string>
#include <iostream>
#include <unordered_map>
#include <grpcpp/grpcpp.h>
#include <google/protobuf/util/json_util.h>

#include "apis/cpp/protocol/v1/login.grpc.pb.h"
#include "apps/utils/strings.hpp"
#include "apps/3rd/json.hpp"
#include "apps/login/defs.hpp"

using namespace std;
using nlohmann::json;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using protocol::login::v1::BasicInfoItem;
using protocol::login::v1::DeviceInfo;

using protocol::login::v1::CommonHeaderReq;
using protocol::login::v1::CommonHeaderResp;

using protocol::login::v1::LoginAPI;
using protocol::login::v1::LoginReq;
using protocol::login::v1::LoginResp;
using protocol::login::v1::LogoutReq;
using protocol::login::v1::LogoutResp;
using protocol::login::v1::RegisterReq;
using protocol::login::v1::RegisterResp;
using protocol::login::v1::UpdateBasicInfoReq;
using protocol::login::v1::UpdateBasicInfoResp;

using google::protobuf::util::MessageToJsonString;
using google::protobuf::util::JsonStringToMessage;

using login::RespCode;

class LoginClient
{
public:
    LoginClient(std::shared_ptr<Channel> channel)
        : stub_(LoginAPI::NewStub(channel))
    {
        reqHandlers = {
            {"register", &LoginClient::onRegister},
            {"login", &LoginClient::onLogin},
            {"update", &LoginClient::onUpdate},
            {"logout", &LoginClient::onLogout},
        };
        printOp.add_whitespace = true;
        printOp.always_print_primitive_fields = true;
        printOp.preserve_proto_field_names = true;
    }

    void Exec(string cmd, string data)
    {
        auto it = reqHandlers.find(cmd);
        if (it == reqHandlers.end())
        {
            cout << "invalid cmd:" << cmd << endl;
            return;
        }
        (this->*(it->second))(data);
    }

private:
    void onRegister(string data)
    {
        RegisterReq req;
        JsonStringToMessage(data, &req, parseOp);
        RegisterResp resp;
        ClientContext ctx;
        Status status = stub_->Register(&ctx, req, &resp);
        string printJ;
        MessageToJsonString(resp, &printJ, printOp);
        cout << "\ngrpc code:" << status.error_code() << " , msg:" << status.error_message() << endl;
        cout << printJ << endl;
    }

    void onLogin(string data)
    {
        LoginReq req;
        auto status = JsonStringToMessage(data, &req, parseOp);
        if(!status.ok()){
            cout << endl << "grpc status code:" << status.error_code() << ", msg:" << status.error_message() << endl;
            return;
        }
        
        ClientContext ctx;
        auto stream = stub_->Login(&ctx, req);
        while(1){
            LoginResp resp;
            if(stream->Read(&resp))
            {
                string printJ;
                MessageToJsonString(resp, &printJ, printOp);
                cout << printJ << endl;                
                auto header = resp.mutable_header();
                if(header->code()!= RespCode::OK){
                    cout << "login error:[code:" << header->code() << " ][msg:" << header->message() << "]\n";
                    stream->Finish();
                    return;
                }
            }else{
                cout << "stream disconnected" << endl; 
                return;
            }

        }
    }

    void onUpdate(string data)
    {
        UpdateBasicInfoReq req;
        JsonStringToMessage(data, &req, parseOp);

        UpdateBasicInfoResp resp;
        ClientContext ctx;
        Status status = stub_->UpdateBasicInfo(&ctx, req, &resp);
        string printJ;
        MessageToJsonString(resp, &printJ, printOp);
        cout << "\ngrpc code:" << status.error_code() << " , msg:" << status.error_message() << endl;
        cout << printJ << endl;
    }

    void onLogout(string data)
    {
        LogoutReq req;
        JsonStringToMessage(data, &req, parseOp);

        LogoutResp resp;
        ClientContext ctx;
        Status status = stub_->Logout(&ctx, req, &resp);
        string printJ;
        MessageToJsonString(resp, &printJ, printOp);
        cout << "\ngrpc code:" << status.error_code() << " , msg:" << status.error_message() << endl;
        cout << printJ << endl;
    }


private:
    using func = void (LoginClient::*)(string);
    google::protobuf::util::JsonPrintOptions printOp;
    google::protobuf::util::JsonParseOptions parseOp;


    unique_ptr<LoginAPI::Stub> stub_;
    unordered_map<string, func> reqHandlers;
};



vector<string> quits = {"exit", "quit", "bye"};
vector<string> helps = {"help", "info", "man"};
string defaultAddr = "127.0.0.1:12345";
string _cliPrompt = "cli> ";
string exitPrompt = "Au revoir.\n";
string helpPrompt = "Usage: cmd json_data   \ncmds={login,register,update,logout}, use admin tool to generate JSON data.\n";
string jsonPrompt = "Need json data input, the admin tool might help\n";


int main(int argc, char **argv)
{
    string endpoint = (argc > 1) ? argv[1] : defaultAddr;
    LoginClient client(grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials()));

    string line, cmd;
    cout << endpoint << endl << helpPrompt;
    while (1)
    {
        std::cout << _cliPrompt;
        std::getline(std::cin, line);
        auto elts = utils::splitByWhitespace(line);
        if (elts.empty())continue;
        string cmd = elts[0];
        utils::toLower(cmd);

        if (utils::in(cmd, quits)) break;
        if (utils::in(cmd, helps)){cout << helpPrompt;continue;}
        if (elts.size() != 2){cout << jsonPrompt;continue;}

        client.Exec(cmd, elts[1]);
    }

    cout << exitPrompt;
    return 0;
}