
#include "simple_scheduler.h"

namespace model_inference_server 
{
SimpleScheduler::SimpleScheduler(const std::shared_ptr<SchedulerConfig> scheduler_config,
    int32_t total_runners,
    BackendInferFunc backend_infer_func): cfg_(scheduler_config), 
                                            total_runners_(total_runners), 
                                            backend_infer_func_(backend_infer_func) {
    LOG(INFO) << __FUNCTION__;
}

SimpleScheduler::~SimpleScheduler() {
    LOG(INFO) << __FUNCTION__;
}

Status 
SimpleScheduler::Enqueue(std::shared_ptr<InferencePayload> &infer_payload) {
    LOG(INFO) << __FUNCTION__;
    return Status::Success;
}

Status 
SimpleScheduler::UpdateBackendInferFunc(BackendInferFunc infer_func) {
    LOG(INFO) << __FUNCTION__;
    return Status::Success;
}

Status 
SimpleScheduler::Start() {
    LOG(INFO) << __FUNCTION__;
    return Status::Success;
}

Status 
SimpleScheduler::Stop() {
    LOG(INFO) << __FUNCTION__;
    return Status::Success;
}
}//namespace model_inference_server 