#pragma once
#include <atomic>
#include <thread>
#include <grpc++/grpc++.h>

#include "../basic_data.h"
#include "../status.h"
#include "grpc_infer_request_context.h"
#include "../../proto/build/grpc_service.grpc.pb.h"

namespace model_inference_server
{
enum class CallStep {
  START = 1,
  PROCESS,
  FINISH,
};

// 抽象接口类，rpc请求handler
class HandlerBase
{
   public:
    virtual Status Start() = 0;
    virtual Status Stop() = 0;
    virtual ~HandlerBase() {}
};

class InferenceCore;

class GrpcServer {
public:
    static Status Create(
        const std::shared_ptr<ServerConfig> server_config,
        InferenceCore* inference_core,
        std::unique_ptr<GrpcServer> *grpc_server);

    ~GrpcServer();

    Status Start();
    Status Stop();

private:
    GrpcServer(const std::shared_ptr<ServerConfig> server_config,
        InferenceCore* inference_core);

    grpc::ServerBuilder grpc_builder_;
    GRPCService::AsyncService service_;

    std::unique_ptr<grpc::Server> grpc_server_;
    std::unique_ptr<grpc::ServerCompletionQueue> infer_cq_;
    std::unique_ptr<HandlerBase> infer_handler_;
    std::unique_ptr<grpc::ServerCompletionQueue> status_cq_;
    std::unique_ptr<HandlerBase> status_handler_;
    std::unique_ptr<grpc::ServerCompletionQueue> health_cq_;
    std::unique_ptr<HandlerBase> health_handler_;

    const std::shared_ptr<ServerConfig> server_cfg_;
    InferenceCore *inference_core_;

    std::atomic<bool> running_;
};

} // namespace model_inference_serve
