#pragma once

#include <cstdint>
#include <string>

namespace model_inference_server 
{
namespace constants 
{
static constexpr int MAX_SCHEDULER_DEPLETION_WAIT_TIME = 120;
static constexpr char kModelConfigPbTxt[] = "config.pbtxt";
static constexpr int MIN_MODEL_REFRESH_INTERVAL = 5;
static constexpr int DEFAULT_SCHEDUELR_QUEUE_SIZE = 4;
static constexpr int MAX_GRPC_MESSAGE_SIZE = INT32_MAX;
static constexpr char kPyTorchLibTorchPlatform[] = "pytorch_libtorch";
static constexpr char kPyTorchLibTorchFilename[] = "model.pt";
static constexpr char kTreeLitePlatform[] = "treelite";
static constexpr char kTreeLiteFilename[] = "model.so";
static constexpr char kTensorRTPlatform[] = "tensorrt";
static constexpr char kTensorRTFilename[] = "model.plan";
static constexpr char kOpenVINOPlatform[] = "openvino";
static constexpr char kOpenVINOFilename[] = "model.xml";
static constexpr char kOnnxRuntimePlatform[] = "onnxruntime";
static constexpr char kOnnxRuntimeFilename[] = "model.onnx";
static constexpr char kTensorflowSavedModelFilename[] = "saved_model.pb";
static constexpr char kTensorflowSavedModelPlatform[] = "tensorflow_savedmodel";
static constexpr char kLightGBMPlatform[] = "lightgbm";
static constexpr char kLightGBMFilename[] = "lightgbm.txt";
static constexpr char kDemoPlatform[] = "demo";
static constexpr char kDemoFilename[] = "model.model";
static constexpr char kUnsupportedPlatform[] = "unsupported_platform";
} // constants

} //model_inference_server