#pragma once

#include <string>
#include <mutex>
#include <vector>
#include <unordered_map>

namespace model_inference_server
{

enum class Platform {
    PLATFORM_UNKNOWN = 0,
    PLATFORM_PYTORCH_LIBTORCH = 1,
    PLATFORM_TENSORFLOW_SAVEDMODEL = 2,
    PLATFORM_TREE_LITE = 3,
    PLATFORM_TENSORRT = 4,
    PLATFORM_OPENVINO = 5,
    PLATFORM_ONNXRUNTIME = 6,
    PLATFORM_LIGHTGBM = 7,
    PLATFORM_DEMO = 8,
};

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
