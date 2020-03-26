#include "hub.hpp"

namespace login
{
HubStats Hub::GetHubStats()
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return HubStats{mapper_.size()};
}

void Hub::AddMsg(std::string userName, shared_ptr<MsgItem> item)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto stream = mapper_.find(userName);
    if (stream != mapper_.end() && stream->second != nullptr)
    {
        stream->second->AddMsgItem(item);
    }
}

// broadcast global notifications
void Hub::Broadcast(shared_ptr<MsgItem> item)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    for (auto &e : mapper_)
    {
        if (e.second != nullptr)
        {
            e.second->AddMsgItem(item);
        }
    }
}

shared_ptr<StreamItem> Hub::GetUser(std::string userName)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    auto iter = mapper_.find(userName);
    if (iter != mapper_.end())
    {
        return iter->second;
    }
    return nullptr;
}

void Hub::AddUser(std::string userName, shared_ptr<StreamItem> item)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    mapper_[userName] = item;
}

void Hub::DelUser(std::string userName)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    mapper_.erase(userName);
}
} // namespace login