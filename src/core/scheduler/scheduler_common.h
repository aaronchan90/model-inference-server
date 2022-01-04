#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "../status.h"
#include "../inference_payload.h"

namespace model_inference_server 
{

using BackendInferFunc = std::function<Status(int32_t, std::vector<std::shared_ptr<InferencePayload>> &)>;

class Scheduler {
public:
    Scheduler() = default;
    virtual ~Scheduler(){}

    virtual Status Enqueue(std::shared_ptr<InferencePayload> &infer_payload) = 0;
    virtual Status UpdateBackendInferFunc(BackendInferFunc infer_func) = 0;
    virtual Status Start() = 0;
    virtual Status Stop() = 0;
};

}// namespace model_inference_server 