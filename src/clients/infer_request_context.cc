#include "infer_request_context.h"

#include <glog/logging.h>

using namespace model_inference_client;

InferRequestContext::InferRequestContext(InferKessClient* client)
    : client_(client) {
}

bool InferRequestContext::AddInferInput(
    const std::string& input_name,
    const uint8_t* input_raw_data,
    uint32_t input_byte_size,
    const std::vector<uint32_t>& input_dims){
    return AddInferInput(input_name, input_raw_data, input_byte_size, input_dims, DataType::TYPE_INVALID);
}

bool InferRequestContext::AddInferInput(
    const std::string& input_name,
    const uint8_t* input_raw_data,
    uint32_t input_byte_size,
    const std::vector<uint32_t>& input_dims,
    DataType data_type) {
    auto request_header = infer_request_.mutable_meta_data();
    request_header->set_batch_size(1);
    auto input = request_header->add_input();
    input->set_name(input_name);
    for (auto& dim : input_dims) {
        input->add_dims(dim);
    }
    input->set_data_type(data_type);
    input->set_batch_byte_size(input_byte_size);
    auto raw_input = infer_request_.add_raw_input();
    raw_input->append(reinterpret_cast<const char*>(input_raw_data), input_byte_size);
    return true;
}

bool InferRequestContext::AddInferInputTensor(
    std::vector<kwai_inference_engine::TensorProto> & tensors){
    if(tensors.empty()){
        return false;
    }
    for(int i = 0;i < tensors.size(); ++i){
        infer_request_.add_tensors()->Swap(&tensors[i]);
    }
    return true;
}

bool InferRequestContext::AddInferInputs(
    const std::vector<std::pair<const uint8_t *, uint32_t>> &inputs,
    const std::vector<std::vector<uint32_t>> &input_shapes,
    const std::vector<std::string> *input_names_ptr){
    if (inputs.size() != input_shapes.size() || (input_names_ptr != nullptr && (*input_names_ptr).size() != inputs.size())) {
        return false;
    }
    for (int i = 0; i < inputs.size(); i++) {
        std::string input_name;
        if (input_names_ptr != nullptr) {
            input_name = (*input_names_ptr)[i];
        } else {
            input_name = "INPUT__" + std::to_string(i);
        }
        if (!AddInferInput(input_name, inputs[i].first, inputs[i].second, input_shapes[i])) {
            return false;
        }
    }
    return true;
}

bool InferRequestContext::RequestInferOuput(
    const std::string& output_name){
    auto request_header = infer_request_.mutable_meta_data();
    auto output = request_header->add_output();
    output->set_name(output_name);
    return true;
}

bool InferRequestContext::Infer(
    const std::string& model_name,
    int64_t model_version){
    static uint64_t request_id = 0;

    infer_request_.set_model_name(model_name);
    infer_request_.set_model_version(model_version);

    auto request_header = infer_request_.mutable_meta_data();
    request_header->set_id(request_id++);

    auto ret = client_->MakeInferRequest(infer_request_, &infer_response_);
    if (!ret) {
        LOG(ERROR) << "Infer timeout";
        return false;
    }
    if (infer_response_.request_status().code() != RequestStatusCode::SUCCESS) {
        LOG(ERROR) << "Failed with request_status: " << infer_response_.request_status().code();
        return false;
    }
    return true;
}

bool InferRequestContext::GetInferOutput(
    uint32_t output_index,
    const uint8_t** output_raw_data,
    uint32_t* output_byte_size,
    std::vector<uint32_t>& output_dims){
    const auto& response_header = infer_response_.meta_data();

    if (static_cast<int32_t>(output_index) >= response_header.output_size()) {
        LOG(ERROR) << "output_index out of bound";
        return false;
    }

    auto& output = response_header.output(output_index);
    auto& raw = output.raw();
    for (auto dim : raw.dims()) {
        output_dims.push_back(dim);
    }

    if (output_index >= infer_response_.raw_output_size()) {
        LOG(ERROR) << "output_index out of bound";
        return false;
    }

    const auto& raw_output = infer_response_.raw_output(output_index);

    *output_raw_data = reinterpret_cast<const uint8_t*>(raw_output.data());
    *output_byte_size = raw_output.size();
    return true;
}

const std::string& InferRequestContext::GetInferServerName() const{
    const auto& status = infer_response_.request_status();
    return status.server_id();
}

const std::string& InferRequestContext::GetInferErrorMessage() const{
    const auto& status = infer_response_.request_status();
    return status.msg();
}

bool InferRequestContext::Clear(){
    infer_request_.Clear();
    infer_response_.Clear();
    return true;
}
