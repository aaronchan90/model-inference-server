#pragma once

#include <atomic>
#include <deque>
#include <thread>
#include <mutex>
#include <exception>
#include <condition_variable>

namespace model_inference_server 
{
template <typename T>
class BlockingQueue
{
public:
    explicit BlockingQueue(size_t capacity) : buffer_(), capacity_(capacity)
    {
        if (capacity <= 0)
        {
            throw std::invalid_argument("queue capacity should be positive");
        }
        broken_ = false;
    }

    ~BlockingQueue()
    {
        breakAll();
    }

    void push(const T &item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pop_event_.wait(lock, [&] { return broken_ || buffer_.size() < capacity_; });
        if (broken_)
        {
            return;
        }
        buffer_.push_back(item);
        push_event_.notify_one();
    }

    void push(T &&item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pop_event_.wait(lock, [&] { return broken_ || buffer_.size() < capacity_; });
        if (broken_)
        {
            return;
        }
        buffer_.push_back(std::move(item));
        push_event_.notify_one();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        push_event_.wait(lock, [&] { return broken_ || buffer_.size() > 0; });
        if (broken_)
        {
            T t;
            return t;
        }

        T item = buffer_.front();
        buffer_.pop_front();
        pop_event_.notify_one();
        return item;
    }

    void pop(T &item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        push_event_.wait(lock, [&] { return broken_ || buffer_.size() > 0; });
        if (broken_)
        {
            item = T();
            return;
        }

        item = buffer_.front();
        buffer_.pop_front();
        pop_event_.notify_one();
    }

    size_t size()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return buffer_.size();
    }

    void breakAll()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        broken_ = true;
        push_event_.notify_all();
        pop_event_.notify_all();
    }

    bool broken()
    {
        return broken_;
    }

private:
    std::mutex mutex_;
    std::condition_variable push_event_;
    std::condition_variable pop_event_;
    std::deque<T> buffer_;
    std::atomic<bool> broken_;
    size_t capacity_;
};

}// namespace model_inference_server 