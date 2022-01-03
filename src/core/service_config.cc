#include <yaml-cpp/yaml.h>
#include <glog/logging.h>

#include "constants.h"
#include "service_config.h"

using namespace model_inference_server;

std::ostream &operator<<(std::ostream &out, const ServerConfig &server_config)
{
    out << "######## Server configuration ########" << std::endl;
    out << "model_repo_path: " << server_config.model_repo_path_ << std::endl;
    out << "grpc_server.addr_port: " << server_config.grpc_server_cfg_.grpc_server_addr_ << std::endl;
    out << "model_control.enable_automatic_refresh: " << server_config.model_control_cfg_.auto_update_ << std::endl;
    out << "model_control.update_interval_: " << server_config.model_control_cfg_.update_interval_ << std::endl;
    out << "model_control.require_warmup_succeed: " << server_config.model_control_cfg_.auto_warmup_ << std::endl;
    out << "scheduler.max_queue_size: " << server_config.scheduler_cfg_.max_queue_size_ << std::endl;
    out << "####################################" << std::endl;
    return out;
}

std::shared_ptr<ServerConfig>
ServiceHelper::LoadServerConfig(const std::string &config_file) {
    LOG(INFO) << "loading server config: " << config_file;
    auto server_config = std::make_shared<ServerConfig>();
    YAML::Node node = YAML::LoadFile(config_file);
    server_config->model_repo_path_ = node["model_repo_path"].as<std::string>();
    
    auto model_control = node["model_control"];
    if (model_control) {
        server_config->model_control_cfg_.auto_update_ = model_control["enable_automatic_refresh"].as<bool>();
        if (server_config->model_control_cfg_.auto_update_ > 0) {
            server_config->model_control_cfg_.update_interval_ = model_control["refresh_interval"].as<int>();
            server_config->model_control_cfg_.update_interval_ = std::max(constants::MIN_MODEL_REFRESH_INTERVAL,
                                                                            server_config->model_control_cfg_.update_interval_);
        }
        server_config->model_control_cfg_.auto_warmup_ = model_control["enable_warmup"].as<bool>();
    }

    auto grpc_server = node["grpc_server"];
    if (grpc_server) {
        server_config->grpc_server_cfg_.grpc_server_addr_ = grpc_server["addr"].as<std::string>("0.0.0.0:50051");
    }

    auto scheduler = node["scheduler"];
    if (scheduler) {
        // TODO
        //server_config->scheduler_cfg_.type_ = scheduler["type"].as<int>(0);
        server_config->scheduler_cfg_.max_queue_size_ = scheduler["max_queue_size"].as<int>(constants::DEFAULT_SCHEDUELR_QUEUE_SIZE);
    }
    LOG(INFO) << "server config: " << *(server_config);
    return server_config;
}
