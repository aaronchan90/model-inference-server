#pragma once

#include <cstdlib>
#include <memory>
//#include <kess/rpc/rpc_facade.h>
//#include "teams/aiplatform/inference_sdk/proto/engine/grpc_service.kess.grpc.pb.h"
//namespace rpc = ks::kess::rpc;

namespace model_inference_client
{
class InferRpcClient
{
public:
    InferKessClient(const rpc::grpc::OptionsForClient& options, int32_t time_out = 100): client_options_(options) {
        // Initialization
        client_ = rpc::RpcFacade::CreateGrpcClient2(client_options_);

        //Delegate the rpc call.
        options_.SetTimeout(std::chrono::milliseconds(time_out));
    }

    bool MakeInferRequest(
        const kwai_inference_engine::InferRequest& request,
        kwai_inference_engine::InferResponse* response) {
        grpc::Status status = client_->All()->SelectOne()->Stub<kwai_inference_engine::kess::GRPCService::Stub>()->Infer(options_, request, response);
        if (!status.ok()) {
            std::cerr << "Status is not OK: %s" << status.error_message().c_str() << std::endl;
            return false;
        }
        return true;
    }
    
    rpc::grpc::Options& GetRpcOptions() {
        return options_;
    }

private:
    std::shared_ptr<rpc::grpc::Client2> client_;
    // kess client options
    const rpc::grpc::OptionsForClient& client_options_;
    // grpc options
    rpc::grpc::Options options_;
};
}  // namespace kwai_infer_front


