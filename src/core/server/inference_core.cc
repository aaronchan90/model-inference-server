#include "../status.h"
#include "inference_core.h"

namespace model_inference_server 
{
InferenceCore::InferenceCore(const std::shared_ptr<ServerConfig>&cfg):service_config_(cfg), running_(false){
    LOG(INFO) << __FUNCTION__;
    auto status = ModelManager::Create(service_config_, &model_manager_);
    if (status != Status::Success){
        THROW_ERROR("Fail to create model manager");
    }
    status = GrpcServer::Create(service_config_, this, &grpc_server_);
    if (status != Status::Success){
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
    if (running_){
        LOG(ERROR) << "inference kernel already running.";
        return Status::Faield_Rpc_Already_Running;
    }
    auto status = grpc_server_->Start();
    if (status != Status::Success) {
        THROW_ERROR("Fail to start grpc server");
    }
    running_ = true;
    return Status::Success;
}

Status
InferenceCore::Stop() {
    if (!running_){
        LOG(ERROR) << "inference kernel already stopped.";
        return Status::Failed_Rpc_Not_Running;
    }
    auto status = grpc_server_->Stop();
    if (status != Status::Success) {
        THROW_ERROR("Fail to stop grpc server");
    } 
    status = model_manager_->Stop();
    if (status != Status::Success) {
        THROW_ERROR("Fail to stop model manager");
    } 
    running_ = false;
    return Status::Success;
}

Status 
InferenceCore::InferAsync(std::shared_ptr<InferencePayload> payload) {
    LOG(INFO) << __FUNCTION__;
    return model_manager_->InferAsync(payload);
}

Status 
InferenceCore::GetStatus(const StatusRequest &request, StatusResponse &response) {
    auto server_status = response.mutable_server_status();
    google::protobuf::Map<std::string, ModelStatus> model_status = std::move(model_manager_->GetModelStatus());
    server_status->mutable_model_status()->swap(model_status);

    if (model_manager_->GetNumberOfLiveModels() > 0){
        server_status->set_ready_state(ServerReadyState::SERVER_READY);
    }else{
        server_status->set_ready_state(ServerReadyState::SERVER_FAILED_TO_INITIALIZE);
    }
    auto request_status = response.mutable_request_status();
    request_status->set_code(RequestStatusCode::SUCCESS);
    // TODO
    //request_status->set_server_id(NetworkUtils::GetHostName());
    return Status::Success;
}

Status 
InferenceCore::GetHealth(const HealthRequest &request, HealthResponse &response) {
    const auto &mode = request.mode();
    auto request_status = response.mutable_request_status();
    if (mode == "live"){
        response.set_health(true);
        request_status->set_code(RequestStatusCode::SUCCESS);
    }else if (mode == "ready"){
        if (model_manager_->GetNumberOfLiveModels() > 0){
            response.set_health(true);
        }else{
            response.set_health(false);
        }
        request_status->set_code(RequestStatusCode::SUCCESS);
    }else{
        request_status->set_code(RequestStatusCode::UNSUPPORTED);
    }
    // TODO
    //request_status->set_server_id(NetworkUtils::GetHostName());
    return Status::Success;
}

}// namespace model_inference_server 
