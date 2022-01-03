#include "../status.h"
#include "inference_core.h"

namespace model_inference_server 
{
    InferenceCore::InferenceCore(const std::shared_ptr<ServerConfig>&cfg):service_config_(cfg), running_(false){
        LOG(INFO) << __FUNCTION__;

        auto status = GrpcServer::Create(service_config_, this, &grpc_server_);
        if (status != Status::Success)
        {
            THROW_ERROR("Fail to create grpc server");
        }
    }

    InferenceCore::~InferenceCore() {
        LOG(INFO) << __FUNCTION__;
        if (running_) {
            Stop();
        }
    }
    
    Status 
    InferenceCore::Start() {
        auto status = grpc_server_->Start();
        if (running_){
            return Status::Faield_Rpc_Already_Running;
        }
        if (status != Status::Success) {
            THROW_ERROR("Fail to start grpc server");
        }
        running_ = true;
        return Status::Success;
    }

    Status
    InferenceCore::Stop() {
        auto status = grpc_server_->Stop();
        if (!running_){
            return Status::Failed_Rpc_Not_Running;
        }
        if (status != Status::Success) {
            THROW_ERROR("Fail to stop grpc server");
        } 
        running_ = false;
        return Status::Success;
    }

    Status 
    InferenceCore::InferAsync(std::shared_ptr<InferencePayload> payload) {
        // TODO
        return Status::Success;
    }

    Status 
    InferenceCore::GetStatus(const StatusRequest &request, StatusResponse &response) {
        // TODO
        return Status::Success;
    }
    Status 
    InferenceCore::GetHealth(const HealthRequest &request, HealthRequest &response) {
        // TODO
        return Status::Success;
    }

}// namespace model_inference_server 