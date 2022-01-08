#include <glog/logging.h>
#include <google/protobuf/text_format.h>

#include "model_manager.h"
#include "../constants.h"
#include "../../utils/file_system/file_utils.h"
#include "../scheduler/simple_scheduler.h"
#include "../backend/backend_common.h"
#include "../backend/demo/demo_backend_factory.h"

using namespace model_inference_server::utils;

namespace model_inference_server 
{

Status
BackendInfo::CreateModelBackend(const std::shared_ptr<ServerConfig> &server_config) {
    LOG(INFO) << __FUNCTION__;
    switch (platform_) {
    case Platform::PLATFORM_DEMO:
        return DemoBackendFactory::CreateBackend(
            server_config,
            model_name_, 
            model_version_,
            path_, 
            model_config_, 
            &backend_
        );
    default:
        LOG(ERROR) << "platform not supported yet.";
        return Status::Failed_Model_Config_Platform_Unknown;
    }
}

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
    // immediately load models as beginning
    LoadModels();

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
                added.emplace_back(model_name, version);
            } else if (it->second->backend_info_->model_version_ != version) {
                updated.emplace_back(model_name, version);
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

    // new added models
    for (const auto &new_model : added) {
        LOG(INFO) << "new_model:" << new_model.model_name_;
        auto backend_info = ModelManager::LoadModel(new_model);
        if (backend_info != nullptr) {
            auto model_run_context = std::make_unique<ModelRunContext>();
            model_run_context->backend_info_ = std::move(backend_info);
            auto backend = model_run_context->backend_info_->backend_.get();
            auto run_func = std::bind(&ModelBackend::Run, backend, std::placeholders::_1, std::placeholders::_2);
            model_run_context->scheduler_.reset(new SimpleScheduler(server_cfg_->scheduler_cfg_, 
                                                                    static_cast<int32_t>(backend->GetInstanceNum()), run_func));
            model_run_context->scheduler_->Start();

            std::lock_guard<std::mutex> lck(model_map_mu_);
            model_map_.insert({new_model.model_name_, std::move(model_run_context)});
            LOG(INFO) << "Model: " << new_model.model_name_ 
                      << " is added, version: " << new_model.model_version_;
        }else {
            LOG(ERROR) << "Fail to load model: " << new_model.model_name_  
                       << " version: " << new_model.model_version_;
        }
    }

    // updated models
    for (const auto &updated_model : updated) {
        LOG(INFO) << "updated_model:" << updated_model.model_name_;
        auto backend_info = ModelManager::LoadModel(updated_model);
        if (backend_info != nullptr) {
            {
                std::lock_guard<std::mutex> lck(model_map_mu_);
                auto it = model_map_.find(updated_model.model_name_);
                if (it != model_map_.end()) {
                    LOG(INFO) << "Model: " << updated_model.model_name_ 
                              << " release old backend " << it->second->backend_info_->model_version_;
                    auto backend = backend_info->backend_.get();
                    auto run_func = std::bind(&ModelBackend::Run, backend, std::placeholders::_1, std::placeholders::_2);
                    it->second->scheduler_->UpdateBackendInferFunc(run_func);
                    it->second->backend_info_.swap(backend_info);
                } else {
                    LOG(ERROR) << "model does not exist: " << updated_model.model_name_ 
                               << " version: " << updated_model.model_version_;
                }
            }
            backend_info.reset();
            LOG(INFO) << "Model: " << updated_model.model_name_ 
                      << " is updated to version: " << updated_model.model_version_;
        }else {
            LOG(ERROR) << "Fail to load model: " << updated_model.model_name_  
                       << " version: " << updated_model.model_version_;
        }
    }

    // deleted models
    for (const auto &deleted_model_name : deleted) {
        LOG(INFO) << "deleted_model_name:" << deleted_model_name;
        std::unique_ptr<ModelRunContext> empty_model_run_context;
        {
            std::lock_guard<std::mutex> lck(model_map_mu_);
            auto it = model_map_.find(deleted_model_name);
            if (it != model_map_.end()) {
                it->second.swap(empty_model_run_context);
                model_map_.erase(it);
            }
        }

        empty_model_run_context.reset();
    }
}

std::unique_ptr<BackendInfo>
ModelManager::LoadModel(const ModelLoadInfo& model_load_info) {
    // TODO check if running
    auto backend_info = std::make_unique<BackendInfo>();
    backend_info->model_name_ = model_load_info.model_name_;
    backend_info->model_version_ = model_load_info.model_version_;
    backend_info->path_ = FileUtils::CombinePath(server_cfg_->model_repo_path_, {backend_info->model_name_});

    auto config_path = FileUtils::CombinePath(server_cfg_->model_repo_path_, {backend_info->model_name_, constants::kModelConfigPbTxt});
    auto config_content = FileUtils::ReadFileText(config_path);
    if (!google::protobuf::TextFormat::ParseFromString(config_content, &(backend_info->model_config_))) {
        LOG(ERROR) << "Fail pase model config " << config_path;
        return nullptr;
    }
    //backend_info->model_filename = backend_info->model_config_.default_model_filename();

    backend_info->platform_ = PlatformHelper::ConvertStringToPlatformType(backend_info->model_config_.platform());
    auto status = backend_info->CreateModelBackend(server_cfg_);
    if (status != Status::Success) {
        LOG(ERROR) << "Fail to create backend for " << backend_info->model_name_;
        return nullptr;
    }
    // TODO warmup model 
    return backend_info;
}

google::protobuf::Map<std::string, ModelStatus> 
ModelManager::GetModelStatus()
{
    LOG_FIRST_N(INFO, 3) << __FUNCTION__;
    google::protobuf::Map<std::string, ModelStatus> model_status;
    std::unique_lock<std::mutex> lck(model_map_mu_);
    for (const auto &kvp : model_map_) {
        const auto &name = kvp.first;
        const auto &backend_info = kvp.second->backend_info_;
        ModelStatus ms;

        // populate version status, currently only the highest version
        ModelVersionStatus model_version_status;
        model_version_status.set_ready_state(ModelReadyState::MODEL_READY);
        auto version_status = ms.mutable_version_status();
        (*version_status)[backend_info->model_version_] = std::move(model_version_status);

        model_status[name] = std::move(ms);
    }
    return model_status;
}

size_t 
ModelManager::GetNumberOfLiveModels() {
    std::lock_guard<std::mutex> lck(model_map_mu_);
    return model_map_.size();
}

void 
ModelManager::UploadLoop() {
    LOG(INFO) << __FUNCTION__
              << " update_interval_: " << server_cfg_->model_control_cfg_.update_interval_;
    while(true) {
        std::unique_lock<std::mutex> lck(cond_mu_);
        cond_var_.wait_for(lck, std::chrono::seconds(server_cfg_->model_control_cfg_.update_interval_), [&]{ return (bool)(!running_); });
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

    const auto& model_name = infer_payload->ModelName();
    auto model_version = infer_payload->ModelVersion();

    std::lock_guard<std::mutex> lck(model_map_mu_);
    auto it = model_map_.find(model_name);
    if (it == model_map_.end()) {
        LOG(ERROR) << "target model backend not found";
        return Status::Failed_Infer_No_Backend_Found;
    }

    return it->second->scheduler_->Enqueue(infer_payload);
}

} // namespace model_inference_server 
