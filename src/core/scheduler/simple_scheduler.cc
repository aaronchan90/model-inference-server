
#include "simple_scheduler.h"

namespace model_inference_server 
{
    SimpleScheduler::SimpleScheduler(const std::shared_ptr<SchedulerConfig> scheduler_config,
        int32_t total_runners,
        BackendInferFunc backend_infer_func): cfg_(scheduler_config), total_runners_(total_runners), backend_infer_func_(backend_infer_func) {

    }

    SimpleScheduler::~SimpleScheduler() {

    }

    Status 
    SimpleScheduler::Enqueue(std::shared_ptr<InferencePayload> &infer_payload) {
        return Status::Success;
    }

    Status 
    SimpleScheduler::UpdateBackendInferFunc(BackendInferFunc infer_func) {
        return Status::Success;
    }

    Status 
    SimpleScheduler::Start() {
        return Status::Success;
    }

    Status 
    SimpleScheduler::Stop() {
        return Status::Success;
    }
}//namespace model_inference_server 