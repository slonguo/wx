#ifndef LOGIN_HUB_HPP
#define LOGIN_HUB_HPP

#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <functional>

#include "defs.hpp"
#include "stream.hpp"

namespace login
{
using std::shared_lock;
using std::shared_mutex;
using std::shared_ptr;
using std::string;
using std::unordered_map;

struct HubStats
{
    size_t streamCount;
};

// Hub is a singleton
class Hub
{
public:
    static Hub &Instance()
    {
        static Hub instance;
        return instance;
    }

    Hub(const Hub &) = delete;
    Hub &operator=(Hub &) = delete;

public:
    HubStats GetHubStats();

    void AddMsg(string, shared_ptr<MsgItem>);
    void Broadcast(shared_ptr<MsgItem>);
    shared_ptr<StreamItem> GetUser(string);
    void AddUser(string, shared_ptr<StreamItem>);
    void DelUser(string);

private:
    Hub() {}

private:
    unordered_map<string, shared_ptr<StreamItem>> mapper_;
    shared_mutex mutex_;
};
} // namespace login

#endif