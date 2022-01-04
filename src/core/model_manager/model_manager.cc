#include <google/protobuf/text_format.h>

#include "../../utils/file_system/file_utils.h"
#include "../constants.h"
#include "model_manager.h"
#include "../../proto/build/model_config.pb.h"

using namespace model_inference_server::utils;

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

ModelManager::ModelManager(const std::shared_ptr<ServerConfig> server_config): server_cfg_(server_config), running_(false) {
    Start();
}

ModelManager::~ModelManager() {
    if (running_) {
        Stop();
    }
}

Status
ModelManager::Start() {
    LOG(INFO) << __FUNCTION__;
    if (running_) {
        return Status::Faield_ModelManager_Already_Running;
    }
    running_ = true;
    if (server_cfg_->model_control_cfg_.auto_update_) {
        update_thd_ = std::thread(&ModelManager::UploadLoop, this);
    }
    return Status::Success;
}

Status 
ModelManager::Stop() {
    LOG(INFO) << __FUNCTION__;
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

int64_t  
ModelManager::GetHighestVersion(const std::string& model_name, 
    const std::string& model_folder, 
    const std::string& model_filename) {

    int64_t highest = -1;
    const auto dirs = FileUtils::GetDirectorySubDirs(FileUtils::CombinePath(server_cfg_->model_repo_path_, {model_name}));
    for (const auto &dir : dirs) {
        try {
            int64_t version = std::stoll(dir);
            if (version > highest) {
                highest = version;
            }
        } catch (std::invalid_argument const &e) {
            LOG(ERROR) << "Invalid version " << dir << " for model " << model_name;
        } catch (std::out_of_range const &e) {
            LOG(ERROR) << "Out-of-range version " << dir << " for model " << model_name;
        }
    }
    
    return highest;
}

/**
 * 检查路径，分类新增、删除、更新的模型文件
*/
void 
ModelManager::GetModelStateUpdate(std::vector<ModelLoadInfo> &added,
    std::vector<ModelLoadInfo> &updated,
    std::vector<std::string> &deleted) {

    const std::string& model_repo_path = server_cfg_->model_repo_path_;

    LOG_FIRST_N(INFO, 3) << __FUNCTION__ 
                         << " model_repo_path: " << model_repo_path;

    std::vector<std::string> cur_models = FileUtils::GetDirectorySubDirs(model_repo_path);
    {
        std::lock_guard<std::mutex> lck(model_map_mu_);
        for (const auto &kv : model_map_) {
            auto& name = kv.first;
            if (std::find(cur_models.begin(), cur_models.end(), name) == cur_models.end()) {
                deleted.push_back(name);
            }
        }
    }

    std::string model_filename;
    for (const auto &model_name : cur_models) {
        auto config_path = FileUtils::CombinePath(model_repo_path, {model_name, constants::kModelConfigPbTxt});
        ModelConfig model_config;
        auto config_content = FileUtils::ReadFileText(config_path);
        if (!google::protobuf::TextFormat::ParseFromString(config_content, &model_config)) {
            LOG(ERROR) << "Fail pase model config " << config_path;
            continue;
        }
        model_filename = model_config.default_model_filename();
        
        std::string model_folder;
        int64_t version = GetHighestVersion(model_name, model_folder, model_filename);
        if (version == -1) {
            LOG(ERROR) << "No valid version found for model " << model_name;
            continue;
        }
        {
            std::lock_guard<std::mutex> lck(model_map_mu_);
            auto it = model_map_.find(model_name);
            if (it == model_map_.end()) {
                added.emplace_back(model_name, model_folder, version);
            } else if (it->second->backend_info_->version_ != version) {
                updated.emplace_back(model_name, model_folder, version);
            }
        }
    }
}

void 
ModelManager::LoadModels() {
    LOG(INFO) << __FUNCTION__;

    std::vector<ModelLoadInfo> added;
    std::vector<ModelLoadInfo> updated;
    std::vector<std::string> deleted;

    GetModelStateUpdate(added, updated, deleted);

    // TODO
    for (const auto &new_model : added) {
        LOG(INFO) << "new_model:" << new_model.model_name_;
    }
    for (const auto &updated_model : updated) {
        LOG(INFO) << "updated_model:" << updated_model.model_name_;
    }
    for (const auto &deleted_model_name : deleted) {
        LOG(INFO) << "deleted_model_name:" << deleted_model_name;
    }
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
