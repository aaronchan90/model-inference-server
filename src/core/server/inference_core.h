#pragma once

#include "grpc_server.h"
#include "../inference_payload.h"
#include "../status.h"
#include "../basic_data.h"
#include "../model_manager/model_manager.h"

namespace model_inference_server
{

class InferenceCore {
public:
    InferenceCore(const std::shared_ptr<ServerConfig> &cfg);
    ~InferenceCore();

    Status Start();
    Status Stop();
    Status InferAsync(std::shared_ptr<InferencePayload> payload);
    Status GetStatus(const StatusRequest &request, StatusResponse &response);
    Status GetHealth(const HealthRequest &request, HealthResponse &response);

private:
    std::shared_ptr<ServerConfig> service_config_;
    std::unique_ptr<ModelManager> model_manager_;
    std::unique_ptr<GrpcServer> grpc_server_;
    std::atomic<bool> running_;
};

} // namespace model_inference_serve