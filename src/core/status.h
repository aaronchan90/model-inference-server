#pragma once

#include <exception>
#include <glog/logging.h>

namespace model_inference_server 
{
enum class Status {
    Success = 0,
    Failed_Generic,
    Faield_Rpc_Already_Running,
    Failed_Rpc_Not_Running,
    Failed_Model_Repo_Path_Not_Exist = 100,
    Failed_Get_GPU_Properties = 200,
    Failed_Infer_No_Scheduler_Configured = 300,
    Failed_Model_Warmup = 400,
};

inline std::ostream &operator<<(std::ostream &out, const Status &status){
    out << "Status = " << static_cast<uint32_t>(status);
    return out;
}

#define RETURN_IF_ERROR(result, message)    \
    do {                                    \
        auto status = result;               \
        if (status != Status::Success) {    \
            LOG(ERROR) << message;          \
            return status;                  \
        }                                   \
    }while(0)

#define RETURN_IF_ERROR_LOG(result, message) \
    do                                       \
    {                                        \
        auto status = result;                \
        if (status != Status::Success)       \
        {                                    \
            LOG(ERROR) << message;           \
            return status;                   \
        }                                    \
    } while (0);

#define RETURN_ERROR(result, message) \
    do                                \
    {                                 \
        LOG(ERROR) << message;        \
        return result;                \
    } while (0);

#define THROW_ERROR(message)               \
    do                                     \
    {                                      \
        throw std::runtime_error(message); \
    } while (0);

#define CHECK_RETURN_ERROR(condition, result, message) \
    do                              \
    {                               \
        if(!(condition))            \
        {                           \
            LOG(ERROR) << message;  \
            return result;          \
        }                           \
    } while (0);
}// namespace model_inference_server 