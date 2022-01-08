#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <deque>
#include <exception>
#include <condition_variable>

#include "../basic_data.h"
#include "scheduler_common.h"
#include "../inference_payload.h"

namespace model_inference_server 
{
template <typename T>
class BlockingQueue
{
public:
    BlockingQueue(size_t capacity):capacity_(capacity),broken_(false),buffer_() {

    }
    ~BlockingQueue() {
        breakAll();
    }
    void push(const T &item) {
        std::unique_lock<std::mutex> lock(mutex_);
        pop_event_.wait(lock, [&] { return broken_ || buffer_.size() < capacity_; });
        if (broken_){
            return;
        }
        buffer_.push_back(item);
        push_event_.notify_one();
    }
    void push(T &&item) {
        std::unique_lock<std::mutex> lock(mutex_);
        pop_event_.wait(lock, [&] { return broken_ || buffer_.size() < capacity_; });
        if (broken_){
            return;
        }
        buffer_.push_back(std::move(item));
        push_event_.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        push_event_.wait(lock, [&] { return broken_ || buffer_.size() > 0; });
        if (broken_){
            T t;
            return t;
        }

        T item = buffer_.front();
        buffer_.pop_front();
        pop_event_.notify_one();
        return item;
    }

    void pop(T &item) {
        std::unique_lock<std::mutex> lock(mutex_);
        push_event_.wait(lock, [&] { return broken_ || buffer_.size() > 0; });
        if (broken_){
            item = T();
            return;
        }

        item = buffer_.front();
        buffer_.pop_front();
        pop_event_.notify_one();
    }
    size_t size() {
        std::unique_lock<std::mutex> lock(mutex_);
        return buffer_.size();
    }

    void breakAll() {
        std::unique_lock<std::mutex> lock(mutex_);
        broken_ = true;
        push_event_.notify_all();
        pop_event_.notify_all();
    }
    bool broken() {
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

class SimpleScheduler : public Scheduler {
public:
    SimpleScheduler(const SchedulerConfig& scheduler_config,
        int32_t total_runners,
        BackendInferFunc backend_infer_func);
    virtual ~SimpleScheduler();
    virtual Status Enqueue(std::shared_ptr<InferencePayload> &infer_payload) override;
    virtual Status UpdateBackendInferFunc(BackendInferFunc infer_func) override;
    virtual Status Start() override;
    virtual Status Stop() override;

private:
    void ProcessLoop(int32_t idx);

    SchedulerConfig cfg_;

    std::mutex infer_func_mu_;
    BackendInferFunc infer_func_;

    BlockingQueue<std::shared_ptr<InferencePayload>> queue_;

    int32_t total_runners_;
    std::vector<std::unique_ptr<std::thread>> threads_;

    std::atomic<bool> running_;
};

} // namespace model_inference_server 