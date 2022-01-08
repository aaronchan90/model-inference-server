#include <vector>
#include "grpc_infer_request_context.h"

namespace model_inference_server 
{
GrpcInferRequestContext::GrpcInferRequestContext(
                            InferRequest *infer_request,
                            InferResponse *infer_response,
                            OnInferCompletedFunc on_infer_completed_func) : infer_request_(infer_request),
                                                                            infer_response_(infer_response),
                                                                            on_infer_completed_func_(on_infer_completed_func),
                                                                            infer_completed_(false) {
        
        auto func = std::bind(&GrpcInferRequestContext::OnInferCompleted, this, std::placeholders::_1, std::placeholders::_2);
        
        std::vector<MemoryReference> inputs_memory;
        for (int idx = 0; idx < infer_request->raw_input_size(); idx++) {
            auto addr = reinterpret_cast<uint8_t *>(const_cast<char *>(infer_request->raw_input(idx).data()));
            inputs_memory.emplace_back(addr, infer_request->raw_input(idx).size());
        }
        infer_payload_.reset(new InferencePayload(infer_request->model_name(), 
                                                  infer_request->model_version(),
                                                  std::move(inputs_memory), 
                                                  &(infer_request->meta_data()), 
                                                  func));
}

void 
GrpcInferRequestContext::OnInferCompleted(
    Status status,
    const std::string &message) {

    if (status == Status::Success) {
        auto request_status = infer_response_->mutable_request_status();

        auto meta_data = infer_response_->mutable_meta_data();
        meta_data->Swap(infer_payload_->GetInferResponseHeader());

        const auto max_output_index = infer_payload_->GetMaxOutputIndex();
        for (int idx = 0; idx <= max_output_index; ++idx) {
            auto output = infer_response_->add_raw_output();
            *output = infer_payload_->GetOutputBuffer(idx);
        }

        //auto request_status = infer_response_->mutable_request_status();
        request_status->set_code(RequestStatusCode::SUCCESS);
        request_status->set_msg(message);
        //request_status->set_server_id(NetworkUtils::GetHostName());
    } else {
        infer_response_->Clear();
        auto request_status = infer_response_->mutable_request_status();
        request_status->set_code(RequestStatusCode::INTERNAL);
        request_status->set_msg(message);
        //request_status->set_server_id(NetworkUtils::GetHostName());
    }
    //SetResponseStatus(status, message);
    on_infer_completed_func_();
}
}//namespace model_inference_server 