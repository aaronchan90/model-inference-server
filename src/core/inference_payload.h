#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

#include "status.h"
#include "memory_misc.h"
#include "../proto/build/api.pb.h"


namespace model_inference_server
{
using OnInferCompletedWithStatusFunc = std::function<void(Status, const std::string &)>;

class InferencePayload {
public:
    InferencePayload() = delete;
    InferencePayload(
        const std::string &model_name,
        int64_t model_version,
        const std::vector<MemoryReference> &inputs_memory,
        const InferRequestHeader *request_header,
        OnInferCompletedWithStatusFunc on_infer_completed_func);

    Status RegisterInferOutput(
        const std::string &name,
        int32_t idx,
        const void *content,
        int32_t content_byte_size,
        const std::vector<int64_t> &shape);

    const std::string &ModelName() const { return model_name_; }

    int64_t ModelVersion() { return model_version_; }

    int32_t GetMaxOutputIndex() const { return max_output_index_; }

    MemoryReference *GetInputMemory(int32_t idx);

    std::string &GetOutputBuffer(int32_t idx);

    void OnInferCompleted(Status status, const std::string &message);
    
    // 记录一系列时间点，统计各个环节耗时
    std::chrono::time_point<std::chrono::high_resolution_clock> recv_time_;
    std::chrono::time_point<std::chrono::high_resolution_clock> dequeue_time_;
    std::chrono::time_point<std::chrono::high_resolution_clock> prepare_input_end_time_;
    std::chrono::time_point<std::chrono::high_resolution_clock> execute_end_time_;
    std::chrono::time_point<std::chrono::high_resolution_clock> output_end_time_;

private:
    void Init();

    std::string model_name_;
    int64_t model_version_;

    const InferRequestHeader* request_header_;
    std::unique_ptr<InferResponseHeader> response_header_;

    std::vector<MemoryReference> inputs_memory_;
    std::unordered_map<uint64_t, std::string> output_buffer_map_;

    OnInferCompletedWithStatusFunc on_infer_completed_func_;

    int32_t max_output_index_ = 0;
};
} // namespace model_inference_server
