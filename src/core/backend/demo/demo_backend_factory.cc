#include <exception>

#include "demo_backend_factory.h"

namespace model_inference_server
{
Status 
DemoBackendFactory::CreateBackend(
    const std::shared_ptr<ServerConfig> &server_config,
    const std::string &model_name,
    int64_t model_version,
    const std::string &path,
    const ModelConfig &model_config,
    std::unique_ptr<ModelBackend> *backend) {
    
    std::unordered_map<std::string, std::string> demo_models;
    try {
        std::unique_ptr<DemoBackend> local_backend(new DemoBackend(
            server_config,
            model_name,
            model_version,
            path,
            model_config,
            demo_models));
        *backend = std::move(local_backend);
        return Status::Success;
    } catch (const std::exception &ex) {
        RETURN_ERROR(Status::Failed_Model_Load, "Create LibTorchBackend failed for model -> '" +
                                                    model_name + "': " + ex.what());
    }
}
} // namespace model_inference_server