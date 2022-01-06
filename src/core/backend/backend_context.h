#pragma once

#include <string>
#include <memory>

namespace model_inference_server 
{
class ModelBackend;
class InferencePayload;

class BackendContext {
public:
    BackendContext(
        const std::string &name,
        const int gpu_device,
        const int max_batch_size): name_(name), gpu_device_id_(gpu_device), max_batch_size_(max_batch_size) {
            
        }

    virtual Status Run(
        const ModelBackend *base,
        std::vector<std::shared_ptr<InferencePayload>> &infer_payloads) = 0;

    virtual ~BackendContext() {}

    static constexpr int NO_GPU_DEVICE = -1;
    static constexpr int NO_BATCHING = 0;

protected:
    std::string name_;
    int gpu_device_id_;
    int max_batch_size_;
    //cudaStream_t cuda_stream_;
};
}