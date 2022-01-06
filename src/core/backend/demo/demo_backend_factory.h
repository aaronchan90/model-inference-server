#pragma once

#include <memory>
#include "demo_backend.h"

namespace model_inference_server 
{

class DemoBackendFactory {
public:
    static Status CreateBackend(
        const std::shared_ptr<ServerConfig> &server_config,
        const std::string &model_name,
        int64_t model_version,
        const std::string &path,
        const ModelConfig &model_config,
        std::unique_ptr<ModelBackend> *backend);
};

} // namespace model_inference_server 