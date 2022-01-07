#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../../proto/build/model_config.pb.h"
#include "../basic_data.h"
#include "../status.h"
#include "../inference_payload.h"
#include "backend_context.h"

namespace model_inference_server
{
class ModelBackend {
public:
    // TODO for debug
    ModelBackend() = default;
    ModelBackend(
        const std::shared_ptr<ServerConfig> &server_config,
        const std::string &model_name,
        int64_t model_version,
        const std::string &path,
        const ModelConfig &model_config,
        Platform platform);

    virtual ~ModelBackend();

    Status GetInput(const std::string &name, const ModelInput **input) const;
    Status GetOutput(const std::string &name, const ModelOutput **output) const;

    int64_t Version() { return model_version_; }
    const std::string &Name() const { return model_name_; }
    const ModelConfig &Config() const { return model_config_; }
    size_t GetInstanceNum() { return contexts_.size(); }

    virtual Status Run(
        int idx,
        std::vector<std::shared_ptr<InferencePayload>> &infer_payloads);
    
    virtual Status RunWarmup(
        int idx,
        std::vector<std::shared_ptr<InferencePayload>> &infer_payloads);

protected:
    Status ProcessModelConfig();
    void WaitInferComplete();

    ModelConfig model_config_;
    std::string model_name_;
    int64_t model_version_;
    Platform platform_;

    std::unordered_map<std::string, ModelInput> input_map_;
    std::unordered_map<std::string, ModelOutput> output_map_;

    std::vector<std::unique_ptr<BackendContext>> contexts_;

    std::atomic<int> running_inference_process_;
};
} // namespace model_inference_server
