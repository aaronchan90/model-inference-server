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

namespace model_inference_server
{

struct BackendInfo {
    std::string name_;
    std::string filename_;
    int64_t last_modified_time_;
    int64_t version_;
    
    std::unique_ptr<ModelBackend> backend_;
};

class ModelRunContext {
public:
    ModelRunContext() {
        // TODO
    }
    ~ModelRunContext() {
        // TODO
    }
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

private:

    struct ModelLoadInfo {
        ModelLoadInfo(const std::string &model_name, const std::string &model_folder, int64_t model_version) :
            model_name_(model_name), model_folder_(model_folder), model_version_(model_version){}

        std::string model_name_;
        std::string model_folder_;
        int64_t model_version_;
    };

    int64_t GetHighestVersion(const std::string& model_name, 
        const std::string& model_folder, 
        const std::string& model_filename);

    void GetModelStateUpdate(std::vector<ModelLoadInfo> &added,
                            std::vector<ModelLoadInfo> &updated,
                            std::vector<std::string> &deleted);

    void LoadModels();

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
