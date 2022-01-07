#include "simple_scheduler.h"
#include "../../utils/timer/timer.h"

using namespace model_inference_server::utils;

namespace model_inference_server 
{
SimpleScheduler::SimpleScheduler(const SchedulerConfig& scheduler_config,
    int32_t total_runners,
    BackendInferFunc backend_infer_func): cfg_(scheduler_config), 
                                            total_runners_(total_runners), 
                                            queue_(scheduler_config.max_queue_size_),
                                            infer_func_(backend_infer_func),
                                            running_(false) {
    LOG(INFO) << __FUNCTION__;
}

SimpleScheduler::~SimpleScheduler() {
    LOG(INFO) << __FUNCTION__;
    Stop();
}

Status 
SimpleScheduler::Enqueue(std::shared_ptr<InferencePayload> &infer_payload) {
    if (!running_) {
        LOG(INFO) << "scheduler already stopped.";
        return Status::Failed_Scheduler_Not_Running;
    }

    queue_.push(infer_payload);
    LOG_FIRST_N(INFO, 3) << __FUNCTION__ 
                         << " q_size: " << queue_.size() 
                         << " thread_id: " << std::this_thread::get_id();

    return Status::Success;
}

Status 
SimpleScheduler::UpdateBackendInferFunc(BackendInferFunc infer_func) {
    LOG(INFO) << __FUNCTION__;

    std::lock_guard<std::mutex> lck(infer_func_mu_);
    infer_func_ = infer_func;

    return Status::Success;
}

Status 
SimpleScheduler::Start() {
    LOG(INFO) << __FUNCTION__;

    if (running_) {
        LOG(ERROR) << "Scheduler already started";
        return Status::Failed_Scheduler_Already_Running;
    }

    running_ = true;

    for (int32_t idx=0;idx<total_runners_;idx++) {
        auto thd = std::make_unique<std::thread>(std::bind(&SimpleScheduler::ProcessLoop, this, idx));
        threads_.push_back(std::move(thd));
    }

    return Status::Success;
}

Status 
SimpleScheduler::Stop() {
    LOG(INFO) << __FUNCTION__;

    if (!running_) {
        LOG(INFO) << "scheduler already stopped.";
        return Status::Failed_Scheduler_Not_Running;
    }

    // wait for already-enqueued-requests to finish
    Timer timer;
    timer.tic();
    while (queue_.size() > 0 && timer.toc() < constants::MAX_SCHEDULER_DEPLETION_WAIT_TIME) {
        LOG(INFO) << "WAIT for all requests processed: " << queue_.size();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    running_ = false;  // Make sure set it false before joining the threads
    queue_.breakAll();

    for (auto& thd : threads_) {
        if (thd->joinable()) {
            thd->join();
        }
    }

    return Status::Success;
}

void 
SimpleScheduler::ProcessLoop(int32_t idx) {
    LOG(INFO) << __FUNCTION__ << "idx:" << idx;

    // TODO 
    while(running_) {
        auto infer_payload = queue_.pop();
        if (infer_payload == nullptr) {
            continue;
        }
        // for simple-scheduler, every payload will be handled seperately.
        std::vector<std::shared_ptr<InferencePayload>> infer_payloads{infer_payload};

        BackendInferFunc infer_func;
        {
            std::lock_guard<std::mutex> lck(infer_func_mu_);
            infer_func = infer_func_;
        }

        std::string message;
        auto status = infer_func_(idx, infer_payloads);
        if (status != Status::Success) {
            LOG(ERROR) << "infer failed";
            std::stringstream ss;
            ss << status;
            message = ss.str();
        }

        infer_payload->OnInferCompleted(status, message);
    }

    return ;
}

}//namespace model_inference_server 