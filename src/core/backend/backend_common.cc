#include <thread>
#include <exception>
#include "backend_common.h"

namespace model_inference_server
{
ModelBackend::ModelBackend(
    const std::shared_ptr<ServerConfig> &server_config,
    const std::string &model_name,
    int64_t model_version,
    const std::string &path,
    const ModelConfig &model_config,
    Platform platform) : model_name_(model_name), 
                         model_version_(model_version),
                         model_config_(model_config),
                         platform_(platform),
                         running_inference_process_(0){
    LOG(INFO) << __FUNCTION__ ;
    auto status = ProcessModelConfig();
    if (status != Status::Success) {
        throw std::runtime_error("ProcessModelConfig() failed");
    }
}

ModelBackend::~ModelBackend() {
    LOG(INFO) << __FUNCTION__ ;
}

Status 
ModelBackend::GetInput(const std::string &name, const ModelInput **input) const {
    const auto it = input_map_.find(name);
    if (it == input_map_.end()) {
        LOG(ERROR) << "io:" << name << " not found";
        return Status::Failed_Model_Config_IO_Not_Found;
    }
    *input = &it->second;
    return Status::Success;
}

Status 
ModelBackend::GetOutput(const std::string &name, const ModelOutput **output) const {
    const auto it = output_map_.find(name);
    if (it == output_map_.end()) {
        LOG(ERROR) << "io:" << name << " not found";
        return Status::Failed_Model_Config_IO_Not_Found;
    }
    *output = &it->second;
    return Status::Success;
}

Status 
ModelBackend::ProcessModelConfig() {
    if (model_config_.platform().empty()) {
        LOG(ERROR) << "model platform not set";
        return Status::Failed_Model_Config_Invalid;
    }
    if (model_config_.instance_group().size() == 0) {
        LOG(ERROR) << "model instance_group not set";
        return Status::Failed_Model_Config_Invalid;
    }
    for (const auto& group : model_config_.instance_group()) {
        if (group.kind() == ModelInstanceGroup::KIND_CPU){
            if (group.gpus().size() > 0) {
                LOG(ERROR) << "model config has kind KIND_CPU but specifies one or more GPUs";
                return Status::Failed_Model_Config_Invalid;
            }
        } else if (group.kind() == ModelInstanceGroup::KIND_GPU) {
            LOG(ERROR) << "KIND_GPU not supported yet";
            return Status::Failed_Model_Config_Invalid;
        }
    }

    for (const auto& io : model_config_.input()) {
        input_map_.insert(std::make_pair(io.name(), io));
    }
    for (const auto& io : model_config_.output()) {
        output_map_.insert(std::make_pair(io.name(), io));
    }
    return Status::Success;
}

void 
ModelBackend::WaitInferComplete() {
    while(running_inference_process_>0) {
        LOG(INFO) << __FUNCTION__ << "inference instance still running, sleep 50ms ...";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

Status 
ModelBackend::Run(
    int idx,
    std::vector<std::shared_ptr<InferencePayload>> &infer_payloads) {
    LOG(INFO) << __FUNCTION__ ;
    running_inference_process_ += 1;
    if (idx < 0 || static_cast<unsigned long>(idx) > contexts_.size()) {
        LOG(ERROR) << "invalid backend context idx:" << idx;
        return Status::Failed_Generic;
    }

    auto context = contexts_[idx].get();
    auto status = context->Run(this, infer_payloads);

    running_inference_process_ -= 1;

    return status;
}
    
Status 
ModelBackend::RunWarmup(
    int idx,
    std::vector<std::shared_ptr<InferencePayload>> &infer_payloads) {
    
    return Run(idx, infer_payloads);
}

} // namespace model_inference_server