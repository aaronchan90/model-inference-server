#pragma once

#include <cstdint>
#include <string>

namespace model_inference_server 
{
namespace constants 
{
static constexpr char kModelConfigPbTxt[] = "config.pbtxt";
static constexpr int MIN_MODEL_REFRESH_INTERVAL = 5;
static constexpr int DEFAULT_SCHEDUELR_QUEUE_SIZE = 4;
static constexpr int MAX_GRPC_MESSAGE_SIZE = INT32_MAX;
} // constants

} //model_inference_server