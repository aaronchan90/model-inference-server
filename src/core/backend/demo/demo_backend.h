#pragma once

#include "../backend_common.h"

namespace model_inference_server 
{

class DemoBackend : public ModelBackend {
public:
    DemoBackend(
        const std::shared_ptr<ServerConfig> &server_config,
        const std::string &model_name,
        int64_t model_version,
        const std::string &path,
        const ModelConfig &model_config,
        const std::unordered_map<std::string, std::string> &models);
    virtual ~DemoBackend();
private:
    Status CreateExecuteContexts(
        const std::unordered_map<std::string, std::string> &models);
    Status CreateExecuteContext(
        const std::string& instance_name,
        const int gpu_device,
        const std::unordered_map<std::string, std::string> &models);

    struct Context: public BackendContext {
        Context(const std::string &name,
                const int gpu_device,
                const int max_batch_size = 0);
        Status Run(
            const ModelBackend *base,
            std::vector<std::shared_ptr<InferencePayload>> &infer_payloads);
        virtual ~Context();
    };
};
} // namespace model_inference_server 