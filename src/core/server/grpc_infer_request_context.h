#pragma once

#include <exception>
#include <string>
#include <utility>

#include "../status.h"
#include "../../proto/build/grpc_service.pb.h"
#include "../inference_payload.h"

namespace model_inference_server 
{
using OnInferCompletedFunc = std::function<void()>;

class GrpcInferRequestContext {
public:
    GrpcInferRequestContext(InferRequest *infer_request,
                            InferResponse *infer_response,
                            OnInferCompletedFunc on_infer_completed_func);

    ~GrpcInferRequestContext(){}

    std::shared_ptr<InferencePayload> &GetInferencePayload() {
        return infer_payload_;
    }

    void OnInferCompleted(
        Status status,
        const std::string &message);

private:
    InferRequest *infer_request_;
    InferResponse *infer_response_;

    std::shared_ptr<InferRequest> processed_request_;
    std::shared_ptr<InferResponse> processed_response_;

    std::shared_ptr<InferencePayload> infer_payload_;

    OnInferCompletedFunc on_infer_completed_func_;

    bool infer_completed_;
};
}// namespace model_inference_server 