#pragma once

#include <cstdint>
#include <vector>

//#include "infer_kess_client.h"
#include "../proto/build/grpc_service.grpc.pb.h"

namespace model_inference_client
{
using DataType = model_inference_server::DataType;
class InferRequestContext
{
   public:
    InferRequestContext(InferKessClient* kess_client);

    /*
    * Aadd tensor input for model inference. Make sure adding input in the same order as their appearance in the model config.
    * 
    * @param input_name: the input name. Should exactly match the input name shown in model
    * @param input_raw_data: pointer of the starting element of the input
    * @param input_byte_size: size of the input in byte, eg., it can be batch_size * 224 * 224 * 3 * sizeof(float)
    * @param input_dims: the input dimensions. For example, for image classification input it can be [batch_size, 224, 224, 3]. 
    * @return true if success
    */
    bool AddInferInput(
        const std::string& input_name,
        const uint8_t* input_raw_data,
        uint32_t input_byte_size,
        const std::vector<uint32_t>& input_dims);
    /*
    * enum DataType {
    *   TYPE_INVALID ,
    *   TYPE_BOOL ,
    *
    *   TYPE_UINT8  ,
    *   TYPE_UINT16 ,
    *   TYPE_UINT32 ,
    *   TYPE_UINT64 ,
    *
    *   TYPE_INT8  ,
    *   TYPE_INT16 ,
    *   TYPE_INT32 ,
    *   TYPE_INT64 ,
    *
    *   TYPE_FP16  ,
    *   TYPE_FP32  ,
    *   TYPE_FP64  ,
    *   }
    */
    bool AddInferInput(
        const std::string& input_name,
        const uint8_t* input_raw_data,
        uint32_t input_byte_size,
        const std::vector<uint32_t>& input_dims,
        DataType data_type);

		/**
     * @brief fill infer_request.tensors
     * for(int i = 0;i < tensors.size(); ++i){
        infer_request_.add_tensors()->Swap(&tensors[i]);
    }
     * @param tensors 
     * @return true 
     * @return false 
     */
    bool AddInferInputTensor(
       std::vector<model_inference_server::TensorProto> & tensors
   );
    /*
    * Add multiple tensor inputs for model inference. Make sure adding input in the same order as their appearance in the model config.
    *
    * @param inputs: the vector of pairs in input data pointer and byte size
    * @param input_shapes: the vector of input dimensions
    * @param input_names_ptr: the pointer of vector of input names. Should exactly match the input name shown in model. If nullptr, default set to {INPUT__0, INPUT__1, ...}
    * @return true if success
    */
    bool AddInferInputs(
        const std::vector<std::pair<const uint8_t *, uint32_t>> &inputs,
        const std::vector<std::vector<uint32_t>> &input_shapes,
        const std::vector<std::string> *input_names_ptr = nullptr);
    /*
    * Request a model output to be returned from predict server. Call mulitple times if multiple outputs should be returned
    * 
    * @param output_name: name of an output to be returned. The name should match the name of corresponding output in the model config
    * @return true if success
    */
    bool RequestInferOuput(
        const std::string& output_name);

    /*
    * Request model inference synchronously
    * 
    * @param model_version: -1 means using the latest version. This is by default
    * @return true if success
    */
    bool Infer(
        const std::string& model_name,
        /*-1 means latest version */
        int64_t model_version = -1);

    /*
    * Get inference result of an output by index
    * 
    * @param output_index: index of output
    * @param output_raw_data: address of ponter of raw data of the output
    * @param output_byte_size: size of the output in byte
    * @param output_dims: dimensions of the output
    * @return true if success
    */
    bool GetInferOutput(
        uint32_t output_index,
        const uint8_t** output_raw_data,
        uint32_t* output_byte_size,
        std::vector<uint32_t>& output_dims);

    /* 
     * Return the host server that does inference for the request
     */
    const std::string& GetInferServerName() const;

    /* 
     * Return inference error message if any
     */
    const std::string& GetInferErrorMessage() const;

    /*
    * You can call this method to clear and reuse this object for multiple inference request
    * 
     * @return true if success
    */
    bool Clear();

   private:
    InferKessClient* client_;

    model_inference_server::InferRequest infer_request_;

    model_inference_server::InferResponse infer_response_;
};
}  // namespace model_inference_client
