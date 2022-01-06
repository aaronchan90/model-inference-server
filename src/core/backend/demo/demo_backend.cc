
#include <glog/logging.h>
#include <exception>
#include "demo_backend.h"

namespace model_inference_server 
{

DemoBackend::DemoBackend(
    const std::shared_ptr<ServerConfig> &server_config,
    const std::string &model_name,
    int64_t model_version,
    const std::string &path,
    const ModelConfig &model_config,
    const std::unordered_map<std::string, std::string> &models) : ModelBackend(server_config,
                                                                                model_name,
                                                                                model_version,
                                                                                path,
                                                                                model_config,
                                                                                Platform::PLATFORM_DEMO) {
    LOG(INFO) << __FUNCTION__;                                                                  
    auto status = CreateExecuteContexts(models);
    if (status != Status::Success) {
        throw std::runtime_error("CreateExecutionContexts failed");
    }  

}

DemoBackend::~DemoBackend() {

}

Status
DemoBackend::CreateExecuteContexts(const std::unordered_map<std::string, std::string> &models) {

    Status status;
    for (const auto &group : model_config_.instance_group()) {
        for (int c = 0; c < group.count(); c++) {
            if (group.kind() == ModelInstanceGroup::KIND_CPU) {
                const std::string instance_name = group.name() + "_" + std::to_string(c) + "_cpu";
                status = CreateExecuteContext(instance_name, 0,  models);
                if (status != Status::Success) {
                    LOG(ERROR) << "CreateExecuteContext failed";
                    return status;
                }
            }else {
                LOG(ERROR) << "gpu not supported yet.";
                return Status::Failed_Model_Config_Invalid;
            }
        }
    }
    return Status::Success;
}

Status
DemoBackend::CreateExecuteContext(
    const std::string& instance_name,
    const int gpu_device,
    const std::unordered_map<std::string, std::string> &models) {
    
    //std::unique_ptr<BackendContext> ctx = std::make_unique<Context>(instance_name,gpu_device,max_batch_size);
    contexts_.emplace_back(new Context(instance_name, gpu_device));
    
    return Status::Success;
}

DemoBackend::Context::Context(const std::string &name,
    const int gpu_device,
    const int max_batch_size) : BackendContext(name,
                                                    gpu_device,
                                                    max_batch_size) {
    LOG(INFO) << __FUNCTION__;
}

DemoBackend::Context::~Context() {
    LOG(INFO) << __FUNCTION__;
}

Status 
DemoBackend::Context::Run(
    const ModelBackend *base,
    std::vector<std::shared_ptr<InferencePayload>> &infer_payloads) {
    LOG(INFO) << __FUNCTION__;
    return Status::Success;
}

} // namespace model_inference_server 