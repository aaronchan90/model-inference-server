//#include <gflags/gflags.h>
//#include <glog/logging.h>

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <utility>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "../proto/build/grpc_service.grpc.pb.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using model_inference_server::GRPCService;
using model_inference_server::StatusRequest;
using model_inference_server::StatusResponse;
using model_inference_server::HealthRequest;
using model_inference_server::HealthResponse;
using model_inference_server::InferRequest;
using model_inference_server::InferResponse;
using model_inference_server::RequestStatusCode;
using model_inference_server::ServerReadyState;

class StatusClient {
public:
    explicit StatusClient(std::shared_ptr<Channel> channel, int timeout) 
        : stub_(GRPCService::NewStub(channel)), timeout_(timeout) {}
    
    bool ModelReady(const std::string& model_name) {
        StatusResponse response;
        StatusRequest request;
        request.set_model_name(model_name);

        ClientContext context;
        auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(timeout_);
        context.set_deadline(deadline);
        auto result = stub_->Status(&context, request, &response);

        if (!result.ok()) {
            std::cout << "GetStatus() timeout" << std::endl;
            return false;
        } else {
            if (response.request_status().code() != RequestStatusCode::SUCCESS) {
                std::cout << "Failed with request_status: " << response.request_status().code() << std::endl;
                return false;
            }

            if (response.server_status().ready_state() != ServerReadyState::SERVER_READY) {
                std::cout << "Server not ready net. ready_state: " << response.server_status().ready_state() << std::endl;
                return false;
            }
        }
        return true;
    }
private:
    std::unique_ptr<GRPCService::Stub> stub_;
    int timeout_;
};

class HealthClient {
public:
    explicit HealthClient(std::shared_ptr<Channel> channel, int timeout) 
        : stub_(GRPCService::NewStub(channel)), timeout_(timeout) {}
    
    bool ProbeReadiness() {
        return MakeHealthRequest("ready");
    }

    bool ProbeLiveness() {
        return MakeHealthRequest("live");
    }

    bool MakeHealthRequest(const std::string& mode) {
        HealthResponse response;
        HealthRequest request;
        request.set_mode(mode);

        ClientContext context;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::seconds(timeout_);
        context.set_deadline(deadline);

        auto result = stub_->Health(&context, request, &response);
        if (!result.ok()) {
            std::cout << "GetHealth() timeout" << std::endl;
            return false;
        }
        if (response.request_status().code() != RequestStatusCode::SUCCESS) {
            std::cout << "Failed with request_status: " << response.request_status().code() << std::endl;
            return false;
        }
        return response.health();
    }

private:
    std::unique_ptr<GRPCService::Stub> stub_;
    int timeout_; 
};


class InferClient {
public:
    explicit InferClient(std::shared_ptr<Channel> channel, int timeout) 
        : stub_(GRPCService::NewStub(channel)), timeout_(timeout), request_id_(0) {}

    bool RequestInference(const std::string &model_name,
                            const std::vector<std::string> &input_names,
                            const std::vector<std::pair<const uint8_t *, uint32_t>> &inputs,
                            const std::vector<std::vector<uint32_t>> &input_shapes,
                            const std::vector<model_inference_server::DataType> &input_dtypes,
                            //std::vector<std::vector<float>> &outputs,
                            model_inference_server::InferResponse &infer_response,
                            //std::vector<std::vector<uint32_t>> *output_shapes = nullptr,
                            int batch_size = 1) {
        
        InferRequest infer_request;
        infer_request.Clear();

        infer_request.set_model_name(model_name);
        infer_request.set_model_version(-1);  // latest
        auto meta_data = infer_request.mutable_meta_data();
        meta_data->set_id(request_id_++);
        meta_data->set_batch_size(batch_size);

        for (int idx = 0; idx < inputs.size(); ++idx) {
            const auto &input_name = input_names[idx];
            auto input = meta_data->add_input();
            //CHECK(idx < MAX_MODEL_INPUT_NUM);
            input->set_name(input_names[idx]);
            if (!input_dtypes.empty()) {
                input->set_data_type(input_dtypes[idx]);
            }
            const auto &input_shape = input_shapes[idx];
            for (auto &dim : input_shape) {
                input->add_dims(dim);
            }

            auto total_byte_size = inputs[idx].second;
            input->set_batch_byte_size(total_byte_size);

            auto raw_input = infer_request.add_raw_input();
            raw_input->append(reinterpret_cast<const char *>(inputs[idx].first), total_byte_size);
        }

        auto grpc_result = RequestInference(infer_request, infer_response);
        if (!grpc_result.ok()) {
            std::cout << "rpc failed" << std::endl;
            return false;
        }
        if (infer_response.request_status().code() != RequestStatusCode::SUCCESS) {
            std::cout << "Failed with request_status: " 
                      << infer_response.request_status().code()
                      << std::endl;
            return false;
        }
        return true;
    }
    
    grpc::Status RequestInference(const model_inference_server::InferRequest &request,
                                        model_inference_server::InferResponse &response) {
        
        grpc::ClientContext context;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_);
        context.set_deadline(deadline);
        auto result = stub_->Infer(&context, request, &response);
        if (!result.ok()) {
            std::cout << "Infer timeout" << std::endl;
            return result;
        }

        return result;
    }
    
private:
    std::unique_ptr<GRPCService::Stub> stub_;
    int timeout_; 
    uint64_t request_id_;
};

int main() {

    std::cout << "client start." << std::endl;

    std::string addr("localhost:50052");
    std::unique_ptr<model_inference_server::GRPCService::Stub> server_stub_;
    auto channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());

    std::string model_name("demo");

    HealthClient hclient(channel, 3);
    auto ret1 = hclient.ProbeLiveness();
    std::cout << ret1 << std::endl;

    StatusClient sclient(channel, 3);
    auto ret2 = sclient.ModelReady(model_name);
    std::cout << ret2 << std::endl;

    InferClient iclient(channel, 3);

    std::vector<std::string> input_names = {"INPUT__0", "INPUT__1"};

    std::vector<uint32_t> input_data0={1,2,3,4};

    std::vector<uint32_t> input_data1={5,6,7,8};

    std::vector<std::pair<const uint8_t*, uint32_t>> inputs;
    uint32_t input_size = 4*sizeof(input_data0[0]);
    inputs.push_back(std::make_pair(reinterpret_cast<const uint8_t*>(input_data0.data()),input_size));
    inputs.push_back(std::make_pair(reinterpret_cast<const uint8_t*>(input_data1.data()),input_size));

    std::vector<std::vector<uint32_t>> input_shapes;
    std::vector<uint32_t> in0 = {1, 16};
    std::vector<uint32_t> in1 = {1, 16};
    input_shapes.push_back(in0);
    input_shapes.push_back(in1);

    std::vector<model_inference_server::DataType> input_dtypes = {model_inference_server::TYPE_INT32, model_inference_server::TYPE_INT32};

    model_inference_server::InferResponse infer_response;


    auto ret3 = iclient.RequestInference(model_name,input_names,inputs,input_shapes,input_dtypes,infer_response);
    std::cout << ret3 << std::endl;

    return 0;
}