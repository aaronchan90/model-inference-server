#pragma once

#include "../basic_data.h"
#include "scheduler_common.h"

namespace model_inference_server 
{

class SimpleScheduler : public Scheduler {
public:
    SimpleScheduler(const std::shared_ptr<SchedulerConfig> scheduler_config,
        int32_t total_runners,
        BackendInferFunc backend_infer_func);
    virtual ~SimpleScheduler();
    virtual Status Enqueue(std::shared_ptr<InferencePayload> &infer_payload) override;
    virtual Status UpdateBackendInferFunc(BackendInferFunc infer_func) override;
    virtual Status Start() override;
    virtual Status Stop() override;

private:
    void ProcessLoop(int32_t idx);

    int32_t total_runners_;

    std::shared_ptr<SchedulerConfig> cfg_;

    BackendInferFunc backend_infer_func_;
    std::mutex mu_;

    BlockingQueue<std::shared_ptr<InferencePayload>> queue_;

    std::vector<std::unique_ptr<std::thread>> threads_;

    std::atomic<bool> running_;

};

} // namespace model_inference_server 