#include <iostream>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "apis/cpp/protocol/v1/login.grpc.pb.h"
#include "apps/utils/strings.hpp"

using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;
using std::vector;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using protocol::login::v1::AdminReq;
using protocol::login::v1::AdminResp;
using protocol::login::v1::LoginAPI;

using utils::CmdMap;

class LoginAdmin
{
public:
    LoginAdmin(std::shared_ptr<Channel> channel)
        : stub_(LoginAPI::NewStub(channel)) {}

    void Exec(string cmd, string f1, string f2)
    {
        AdminReq req;
        req.set_cmd(cmd);
        req.set_f1(f1);
        req.set_f2(f2);

        AdminResp resp;
        ClientContext ctx;
        Status status = stub_->AdminOp(&ctx, req, &resp);

        if (status.ok())
        {
            cout << "[header]code:" << resp.mutable_header()->code() << ", msg:" + resp.mutable_header()->message() << endl;
            cout << "[body]" << endl << resp.result() << endl;
        }
        else
        {
            cout << "resp abnormal" << endl;
            cout << "code:" << status.error_code() << ": " << status.error_message() << endl;
            cout << status.error_details() << endl;
        }
    }

private:
    unique_ptr<LoginAPI::Stub> stub_;
};

CmdMap cmdMap = {
    {"config", {
                   {"set    json_config", "eg:config set {\"kick_mode\":1}"},
                   {"show              ", "show config info(db user/passwd obscured)"},
               }},
    {"mock", {
                 {"login    user_name", "eg: mock login minkovsky"},
                 {"logout   user_name", "eg: mock logout alexia"},
                 {"register user_name", "eg: mock register bob"},
                 {"update   user_name", "eg: mock update isaac"},
             }},
    {"inspect", {
                    {"(no args)", "show internal stats"},
                }},
    {"broadcast", {{"msg", "broadcast a message to test grpc stream."}}},
    {"getuser", {{"user_name", "get user basic info."}}},
    {"help", {
                 {"(no args)", "`help` can also be `man`, `info`"},
                 {"specific_cmd", "help info on specific command"},
             }},
    {"exit", {
                 {"(no args)", "exit program. `exit` can also be `quit`, `bye`"},
             }},
};

vector<string> quits = {"exit", "quit", "bye"};
vector<string> helps = {"help", "info", "man"};
string adminPrompt = "admin> ";
string _exitPrompt = "Au revoir.\n";
string defaultAddr = "127.0.0.1:12345";
string _helpPrompt = "type `help` for admin commands info\n";
string errorPrompt = "Invalid command, letters(a-zA-Z) only. Try `help` for more.\n";

int main(int argc, char **argv)
{
    string endpoint = (argc > 1) ? argv[1] : defaultAddr;
    LoginAdmin admin(grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials()));
    cout << endpoint << endl << _helpPrompt;

    string line, cmd;
    while (1)
    {
        std::cout << adminPrompt;
        std::getline(std::cin, line);
        auto elts = utils::splitByWhitespace(line);
        if (elts.empty()) continue;
        string cmd = elts[0];
        utils::toLower(cmd);

        if (!utils::isWord(cmd)){cout << errorPrompt;continue;}
        if (utils::in(cmd, helps))
        {
            string specificCmd;
            if (elts.size() > 1 && utils::isWord(elts[1]))
            {
                specificCmd = elts[1];
                utils::toLower(specificCmd);
            }
            utils::help(cmdMap, specificCmd);
            continue;
        }

        if (utils::in(cmd, quits)) break;
        if (cmdMap.find(cmd) == cmdMap.end()){utils::help(cmdMap);continue;}

        // valid command. send request
        string s1, s2;
        if (elts.size() == 2){s1 = elts[1];}
        else if (elts.size() > 2){s1 = elts[1], s2 = elts[2];}

        admin.Exec(cmd, s1, s2);
    }

    cout << _exitPrompt;
    return 0;
}