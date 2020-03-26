#ifndef UTILS_CONN_POOL_HPP
#define UTILS_CONN_POOL_HPP

#include <mutex>
#include <deque>
#include <set>

#include "apps/3rd/sql/mysqlplus.h"
#include "apps/3rd/loguru.hpp"

// reference:
// https://github.com/active911/connection-pool

namespace utils
{
using daotk::mysql::connect_options;
using daotk::mysql::connection;
using std::deque;
using std::make_shared;
using std::mutex;
using std::scoped_lock;
using std::set;
using std::shared_ptr;

using utils::DBConfig;
struct ConnectionPoolFull : std::exception
{
    char const *what() const throw()
    {
        return "connection pool full";
    };
};

struct ConnectionAllocationFail : std::exception
{
    char const *what() const throw()
    {
        return "connection allocation fail";
    };
};

struct PoolStats
{
    size_t poolSize;
    size_t borrowedSize;
};

class ConnPool
{
public:
    ConnPool(){};
    ConnPool(const ConnPool &) = delete;
    ConnPool &operator=(ConnPool &) = delete;

    bool Init(DBConfig dc)
    {
        conf_ = dc;
        while (pool_.size() < conf_.pool_size)
        {
            shared_ptr<connection> c = make_shared<connection>(dc.conn);
            if (!*c.get())
            {
                // seems brutal
                return false;
            }

            pool_.push_back(c);
        }
        return true;
    }

    PoolStats getStats()
    {
        scoped_lock lock(mutex_);
        PoolStats stats;
        stats.poolSize = pool_.size();
        stats.borrowedSize = borrowed_.size();
        return stats;
    }

    shared_ptr<connection> get()
    {
        scoped_lock lock(mutex_);
        if (pool_.empty())
        {
            for (auto it = borrowed_.begin(); it != borrowed_.end(); ++it)
            {
                // seems redundant. in normal cases this won't be executed
                // unique() is to be deprecated in c++ 20, use use_count
                // https://zh.cppreference.com/w/cpp/memory/shared_ptr/use_count
                if (it->use_count() == 1)
                {
                    LOG_F(INFO, "use_count=1 for pointer");
                    if (*it->get())
                    {
                        return *it;
                    }

                    try
                    {
                        shared_ptr<connection> c = make_shared<connection>(conf_.conn);
                        if (!*c.get())
                        {
                            throw ConnectionAllocationFail();
                        }
                        borrowed_.erase(it);
                        borrowed_.insert(c);
                        return c;
                    }
                    catch (const std::exception &e)
                    {
                        throw ConnectionPoolFull();
                    }
                }
            }

            throw ConnectionPoolFull();
        }
        auto c = pool_.front();
        pool_.pop_front();
        borrowed_.insert(c);

        return c;
    }

    void release(shared_ptr<connection> c)
    {
        scoped_lock lock(mutex_);
        pool_.push_back(c);
        borrowed_.erase(c);
    }

private:
    DBConfig conf_;
    deque<shared_ptr<connection>> pool_;
    set<shared_ptr<connection>> borrowed_;
    mutex mutex_;
};

} // namespace utils

#endif