#pragma once

#include <string>
#include <mutex>
#include <vector>
#include <unordered_map>

namespace model_inference_server
{

enum ExitSignal {
    NoSignal = 0,
    GraceExit,
    ImmediateExit
};

// 一些全局配置
class ModelControlConfig {
public:
    bool auto_update_;
    int update_interval_;
    bool auto_warmup_;
    std::unordered_map<std::string, int> specific_model_version_;
};

class GrpcServerConfig {
public:
    std::string grpc_server_addr_;
};

enum class SchedulerType {
    SCHEDULER_SIMPLE = 0,
    SCHEDULER_BATCH
};

class SchedulerConfig {
public:
    SchedulerType type_;
    int32_t max_queue_size_;
};

class ServerConfig {
public:
    std::string model_repo_path_;
    ModelControlConfig model_control_cfg_;
    GrpcServerConfig grpc_server_cfg_;
    SchedulerConfig scheduler_cfg_;
};

} // namespace model_inference_server
