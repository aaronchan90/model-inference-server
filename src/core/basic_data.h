#pragma once

#include <string>
#include <mutex>
#include <vector>
#include <cstring>
#include <unordered_map>

#include "constants.h"
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

class PlatformHelper
{
public:
static Platform ConvertStringToPlatformType(const std::string &platform){
    if (strcmp(platform.c_str(), constants::kPyTorchLibTorchPlatform) == 0) {
        return Platform::PLATFORM_PYTORCH_LIBTORCH;
    } else if (strcmp(platform.c_str(), constants::kTensorflowSavedModelPlatform) == 0) {
        return Platform::PLATFORM_TENSORFLOW_SAVEDMODEL;
    } else if (strcmp(platform.c_str(), constants::kTreeLitePlatform) == 0) {
        return Platform::PLATFORM_TREE_LITE;
    } else if (strcmp(platform.c_str(), constants::kTensorRTPlatform) == 0) {
        return Platform::PLATFORM_TENSORRT;
    } else if (strcmp(platform.c_str(), constants::kOpenVINOPlatform) == 0) {
        return Platform::PLATFORM_OPENVINO;
    } else if (strcmp(platform.c_str(), constants::kOnnxRuntimePlatform) == 0) {
        return Platform::PLATFORM_ONNXRUNTIME;
    } else if (strcmp(platform.c_str(), constants::kLightGBMPlatform) == 0) {
        return Platform::PLATFORM_LIGHTGBM;
    } else if (strcmp(platform.c_str(), constants::kDemoPlatform) == 0) {
        return Platform::PLATFORM_DEMO;
    } else {
        return Platform::PLATFORM_UNKNOWN;
    }
}

static const char *ConvertPlatformTypeToString(Platform platform){
    switch (platform) {
        case Platform::PLATFORM_PYTORCH_LIBTORCH:
            return constants::kPyTorchLibTorchPlatform;
        case Platform::PLATFORM_TENSORFLOW_SAVEDMODEL:
            return constants::kTensorflowSavedModelPlatform;
        case Platform::PLATFORM_OPENVINO:
            return constants::kOpenVINOPlatform;
        case Platform::PLATFORM_TREE_LITE:
            return constants::kTreeLitePlatform;
        case Platform::PLATFORM_TENSORRT:
            return constants::kTensorRTPlatform;
        case Platform::PLATFORM_ONNXRUNTIME:
            return constants::kOnnxRuntimePlatform;
        case Platform::PLATFORM_LIGHTGBM:
            return constants::kLightGBMPlatform;
        case Platform::PLATFORM_DEMO:
            return constants::kDemoPlatform;
        default:
            return constants::kUnsupportedPlatform;
    }
}
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
