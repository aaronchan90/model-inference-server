#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "../status.h"
#include "../basic_data.h"
#include "../backend/backend_common.h"
#include "../scheduler/scheduler_common.h"
#include "../../proto/build/model_config.pb.h"
#include "../../proto/build/server_status.pb.h"

namespace model_inference_server
{

struct BackendInfo {
    std::string model_name_;
    std::string filename_;
    std::string path_;
    Platform platform_;
    int64_t model_version_;
    ModelConfig model_config_;
    std::unique_ptr<ModelBackend> backend_;

    Status CreateModelBackend(const std::shared_ptr<ServerConfig> &server_config);
};

class ModelRunContext {
public:
    std::unique_ptr<Scheduler> scheduler_;
    std::unique_ptr<BackendInfo> backend_info_;
};

class ModelManager {
public:
    static Status Create(const std::shared_ptr<ServerConfig> server_config,
        std::unique_ptr<ModelManager> *model_manager);

    ModelManager(const std::shared_ptr<ServerConfig> server_config);
    ~ModelManager();
    
    Status Start();
    Status Stop();

    Status InferAsync(std::shared_ptr<InferencePayload> &infer_payload);

    size_t GetNumberOfLiveModels();
    google::protobuf::Map<std::string, ModelStatus> GetModelStatus();

private:

    struct ModelLoadInfo {
        ModelLoadInfo(const std::string &model_name, int64_t model_version) :
            model_name_(model_name), model_version_(model_version){}

        std::string model_name_;
        //std::string model_folder_;
        int64_t model_version_;
    };

    int64_t GetHighestVersion(const std::string& model_name, 
        const std::string& model_folder, 
        const std::string& model_filename);

    void GetModelStateUpdate(std::vector<ModelLoadInfo> &added,
                            std::vector<ModelLoadInfo> &updated,
                            std::vector<std::string> &deleted);

    void LoadModels();

    // generate new ModelRunCtx
    std::unique_ptr<BackendInfo> LoadModel(const ModelLoadInfo& model_load_info);

    void UploadLoop();

    std::thread update_thd_;
    std::shared_ptr<ServerConfig> server_cfg_;
    std::unordered_map<std::string, std::unique_ptr<ModelRunContext>> model_map_;
    std::mutex model_map_mu_;

    std::condition_variable cond_var_;
    std::mutex cond_mu_;

    std::atomic<bool> running_;
};

}// namespace model_inference_serve
