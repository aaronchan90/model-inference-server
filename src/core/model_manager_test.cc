#include <thread>
#include <chrono>

#include "basic_data.h"
#include "status.h"
#include "model_manager/model_manager.h"


using namespace model_inference_server;

int main() {

    std::shared_ptr<ServerConfig> cfg(new ServerConfig());

    cfg->model_repo_path_ = "/root/models/";
    cfg->model_control_cfg_.auto_update_ = true;
    cfg->model_control_cfg_.update_interval_ = 10;

    std::unique_ptr<ModelManager> mm;
    auto status = ModelManager::Create(cfg, &mm);

    std::this_thread::sleep_for(std::chrono::seconds(60));

    status = mm->Stop();

    return 0;
}