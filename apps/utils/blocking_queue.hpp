#ifndef BLOCKING_QUEUE_HPP
#define BLOCKING_QUEUE_HPP

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace utils
{

// a simple thread-safe blocking queue implementation with no size limit.
// reference:
// https://gist.github.com/PolarNick239/f727c0cd923398dc397a05f515452123
// https://stackoverflow.com/questions/19143228/creating-a-standard-map-that-is-thread-safe
// https://stackoverflow.com/questions/30586932/unable-to-add-elements-to-thread-safe-locking-queue-of-shared-pointers

using std::move;
using std::scoped_lock;
using std::unique_lock;
using std::mutex;
using std::condition_variable;

template <typename T, typename Container = std::queue<T>>
class BlockingQueue
{
public:
    using container_type = Container;
    using value_type = typename Container::value_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using mutex_type = mutex;
    using cv_type = condition_variable;

public:
    BlockingQueue() = default;
    BlockingQueue(const BlockingQueue &) = delete;
    BlockingQueue &operator=(const BlockingQueue &) = delete;

    void pop(reference val)
    {
        unique_lock<mutex_type> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty(); });
        val = move(queue_.front());
        queue_.pop();
    }

    void push(const value_type &val)
    {
        scoped_lock<mutex_type> lock(mutex_);
        queue_.push(val);
        cond_.notify_one();
    }

    void push(value_type &&val)
    {
        scoped_lock<mutex_type> lock(mutex_);
        queue_.push(move(val));
        cond_.notify_one();
    }

private:
    Container queue_;
    mutable mutex_type mutex_;
    cv_type cond_;
};

} // namespace utils

#endif //BLOCKING_QUEUE_HPP
