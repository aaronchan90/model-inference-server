#include "inference_payload.h"

namespace model_inference_server 
{

InferencePayload::InferencePayload(
    const std::string &model_name,
    int64_t model_version,
    const std::vector<MemoryReference> &inputs_memory,
    const InferRequestHeader *request_header,
    OnInferCompletedWithStatusFunc on_infer_completed_func) : model_name_(model_name), 
                                                              model_version_(model_version), 
                                                              request_header_(request_header),
                                                              inputs_memory_(inputs_memory),
                                                              on_infer_completed_func_(on_infer_completed_func) {
    Init();
}

void 
InferencePayload::Init() {
    if (request_header_ == nullptr) {
        throw std::invalid_argument("request_header_ should not be null");
    }
    response_header_.reset(new InferResponseHeader());
}

Status
InferencePayload::RegisterInferOutput(
    const std::string &name,
    int32_t idx,
    const void *content,
    int32_t content_byte_size,
    const std::vector<int64_t> &shape){

    LOG_FIRST_N(INFO, 3) << __FUNCTION__ 
                         << " idx: " << idx 
                         << " content_byte_size: " << content_byte_size;

    auto output = response_header_->add_output();
    output->set_name(name);
    auto raw = output->mutable_raw();
    for (auto &dim : shape) {
        raw->add_dims(dim);
    }
    raw->set_batch_byte_size(content_byte_size);

    std::string buffer((const char *)content, content_byte_size);
    output_buffer_map_[idx] = std::move(buffer);
    max_output_index_ = std::max(max_output_index_, idx);
    return Status::Success;
}

MemoryReference*
InferencePayload::GetInputMemory(int32_t idx) {
    if (idx<0 || static_cast<size_t>(idx) >= inputs_memory_.size()) {
        LOG(ERROR) << "idx=" << idx 
                   << " beyond size of inputs " << inputs_memory_.size();
        return nullptr;
    }
    return &inputs_memory_[idx];
}

std::string&
InferencePayload::GetOutputBuffer(int32_t idx) {
    if (idx > max_output_index_) {
        LOG(ERROR) << "idx=" << idx 
                   << " beyond range of outputs " << max_output_index_;
        throw std::invalid_argument("idx beyond range of outputs");
    }
    return output_buffer_map_[idx];
}

void 
InferencePayload::OnInferCompleted(Status status, const std::string &message) {
    on_infer_completed_func_(status, message);
}


} // namespace model_inference_server 