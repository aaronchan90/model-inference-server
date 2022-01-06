#include <thread>
#include <chrono>
#include <iostream>
#include <google/protobuf/text_format.h>

#include "basic_data.h"
#include "status.h"
#include "../../utils/file_system/file_utils.h"
#include "../model_manager/model_manager.h"
#include "../backend/demo/demo_backend_factory.h"
#include "../../proto/build/grpc_service.pb.h"

using namespace model_inference_server;
using namespace model_inference_server::utils;

void MockOnInferComplete(    
    Status status,
    const std::string &message) {
    std::cout << __FUNCTION__ << std::endl;
}

int main() {

    std::shared_ptr<ServerConfig> cfg(new ServerConfig());

    cfg->model_repo_path_ = "/root/models/";
    cfg->model_control_cfg_.auto_update_ = true;
    cfg->model_control_cfg_.update_interval_ = 10;

    auto backend = std::make_unique<ModelBackend>();

    std::string model_name = "demo";
    int64_t model_version = 0;
    std::string path = "/root/workspace/model-inference-server/demo/model_repo/demo";

    std::string config_path = "/root/workspace/model-inference-server/demo/model_repo/demo/config.pbtxt";

    ModelConfig model_config;
    auto config_content = FileUtils::ReadFileText(config_path);
    if (!google::protobuf::TextFormat::ParseFromString(config_content, &model_config)) {
        std::cout << "Fail pase model config " << config_path << std::endl;
        return -1;
    }

    auto status = DemoBackendFactory::CreateBackend(
        cfg,
        model_name, 
        model_version,
        path, 
        model_config, 
        &backend
    );

    if (status != Status::Success) {
        std::cout << "create backend failed" << std::endl;
        return -1;
    }

    int idx = 0;

    std::vector<std::shared_ptr<InferencePayload>> infer_payloads;

    std::vector<MemoryReference> inputs_memory;

    InferRequest infer_request;

    auto func = std::bind(&MockOnInferComplete, std::placeholders::_1, std::placeholders::_2);

    new InferencePayload(model_name, 
                         model_version,
                         std::move(inputs_memory), 
                         &(infer_request.meta_data()), 
                         func);

    status = backend->Run(idx, infer_payloads);
    if (status != Status::Success) {
        std::cout << "backend run failed" << std::endl;
    }

    return 0;
}