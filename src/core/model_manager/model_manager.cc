#include "model_manager.h"


namespace model_inference_server 
{

Status
ModelManager::Create(const std::shared_ptr<ServerConfig> server_config,
    std::unique_ptr<ModelManager> *model_manager) {
    
    LOG(INFO) << __FUNCTION__;
    
    if (server_config == nullptr) {
        return Status::Failed_Invalid_Server_Config;
    }
    // TODO check if model_controller_cfg valid;
    // check if model_repo_dir valid

    *model_manager = std::make_unique<ModelManager>(server_config);
    return Status::Success;
}

ModelManager::ModelManager(const std::shared_ptr<ServerConfig> server_config): server_cfg_(server_config) {
    Start();
}

ModelManager::~ModelManager() {
    if (running_) {
        Stop();
    }
}

Status
ModelManager::Start() {
    if (running_) {
        return Status::Faield_ModelManager_Already_Running;
    }
    running_ = true;
    if (server_cfg_->model_control_cfg_.auto_update_) {
        update_thd_ = std::thread(&ModelManager::UploadLoop,this);
    }
    return Status::Success;
}

Status 
ModelManager::Stop() {
    running_ = false;
    
    {
        std::unique_lock<std::mutex> lck(cond_mu_);
        cond_var_.notify_all();
    }

    if (update_thd_.joinable()) {
        update_thd_.join();
    }

    std::unique_lock<std::mutex> lck1(model_map_mu_);
    model_map_.clear();
    return Status::Success;
}

void 
ModelManager::GetModelStateUpdate(const std::vector<std::string> &cur_models,
    std::vector<ModelLoadInfo> &added,
    std::vector<ModelLoadInfo> &updated,
    std::vector<std::string> &deleted) {

    // TODO
}

void 
ModelManager::LoadModels() {
    LOG(INFO) << __FUNCTION__;

    // TODO
}

void 
ModelManager::UploadLoop() {
    LOG(INFO) << __FUNCTION__
              << " update_interval_: " << server_cfg_->model_control_cfg_.update_interval_;
    while(true) {
        std::unique_lock<std::mutex> lck(cond_mu_);
        cond_var_.wait_for(lck, std::chrono::seconds(server_cfg_->model_control_cfg_.update_interval_), [&]{ return (bool)!running_; });
        if (running_) {
            LoadModels();
        }else {
            LOG(INFO) << "ModelManager::UploadLoop exited";
            break;
        }
    }
}

Status
ModelManager::InferAsync(std::shared_ptr<InferencePayload> &infer_payload) {
    return Status::Success;
}

} // namespace model_inference_server 
