#ifndef LOGIN_STREAM_HPP
#define LOGIN_STREAM_HPP

#include <string>
#include <queue>

#include "defs.hpp"
#include "apps/utils/blocking_queue.hpp"

namespace login
{
using std::shared_ptr;
using std::string;
using utils::BlockingQueue;

class StreamItem
{
public:
    StreamItem(const string name) : userName(name) {}
    ~StreamItem() {}

    StreamItem(const StreamItem &) = delete;
    StreamItem &operator=(StreamItem &) = delete;

    shared_ptr<MsgItem> GetMsgItem()
    {
        shared_ptr<MsgItem> p;
        bq_.pop(p);
        return p;
    }

public:
    void AddMsgItem(shared_ptr<MsgItem> item)
    {
        bq_.push(item);
    }

private:
    string userName;

    // TODO(slonguo): We use an in-memory queue here for simplification.
    // we can use Redis BRPOPLPUSH command https://redis.io/commands/brpoplpush for decoupling, persistence and robustness etc.
    // Persistence and stability matters. Sohpisticated queues are needed in production.
    BlockingQueue<shared_ptr<MsgItem>> bq_;
};
} // namespace login

#endif