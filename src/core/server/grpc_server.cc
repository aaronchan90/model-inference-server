#include <thread>
#include <string>
#include <memory>

#include "grpc_server.h"
#include "grpc_infer_request_context.h"
#include "../constants.h"

namespace model_inference_server
{

template <typename AsyncServiceType, typename RequestType, typename ResponseType>
class Handler : public HandlerBase
{
public:
    class HandlerState{
    public:
        explicit HandlerState(CallStep step = CallStep::START){
            Reset(step);
        }
        ~HandlerState() {
            Release();
        }

        void Reset(CallStep step) {
            unique_id_ = 0;
            ctx_.reset(new grpc::ServerContext());
            responder_.reset(new grpc::ServerAsyncResponseWriter<ResponseType>(ctx_.get()));
            step_ = step;
            request_.Clear();
            response_.Clear();
            grpc_infer_request_context_ = nullptr;
        }

        void Release() {
            ctx_.reset();
            responder_.reset();
        }

        uint64_t unique_id_; // 标记一个request 的唯一id
        RequestType request_; // 注册到grpc async函数中去自动获取内容
        ResponseType response_;
        
        std::unique_ptr<grpc::ServerContext> ctx_;
        std::unique_ptr<grpc::ServerAsyncResponseWriter<ResponseType>> responder_;
        std::shared_ptr<GrpcInferRequestContext> grpc_infer_request_context_;

        CallStep step_;
    };

    HandlerState* StateNew(CallStep step = CallStep::START) {
        HandlerState *state = nullptr;
        state = new HandlerState(step);
        return state;
    }

    void StateRelease(HandlerState* state) {
        delete state;
    }

    Handler(AsyncServiceType *service,
        grpc::ServerCompletionQueue *cq,
        InferenceCore *inference_core) : service_(service), cq_(cq), inference_core_(inference_core) {}
    ~Handler() {}

    virtual Status Start() override {
        std::cout << __FUNCTION__ << std::endl;
        thd_ = std::thread(&Handler::HandleEvents, this);
    }

    virtual Status Stop() override {
        if (thd_.joinable()) {
            thd_.join();
        }
    }

    // main loop
    void HandleEvents() {
        StartNewRequest();
        void* tag;
        bool ok ; 
        while(cq_->Next(&tag, &ok)) {
            HandlerState *state = static_cast<HandlerState *>(tag);
            Process(state, ok);
        }
    }

    // 根据state->step状态值采取不同操作
    void Process(HandlerState *state, bool rpc_ok) {
        const bool shutdown = (!rpc_ok && (state->step_ == CallStep::START));
        if (rpc_ok == 0 && !shutdown) {
            std::cout << "rpc_ok false, state: " << static_cast<int>(state->step_) << ", likley client timeout" << std::endl;
        }

        if (shutdown) {
            state->step_ = CallStep::FINISH;
        }
        if (state->step_ == CallStep::START) {
            StartNewRequest();
            state->step_ = CallStep::PROCESS;
            TakeAction(state);
        }else if (state->step_ == CallStep::FINISH) {
            StateRelease(state);
        }
    }

    // impl by specific handler class
    virtual void TakeAction(HandlerState *state) = 0;
    virtual void StartNewRequest() = 0;

protected:
    AsyncServiceType* service_;
    grpc::ServerCompletionQueue* cq_;
    std::thread thd_;
    InferenceCore* inference_core_;
};

class InferHandler : public Handler<GRPCService::AsyncService, InferRequest, InferResponse> {
public:
    InferHandler(GRPCService::AsyncService* svc, grpc::ServerCompletionQueue* cq, InferenceCore *inference_core) : Handler(svc,cq,inference_core) {}
    virtual ~InferHandler() {}

    virtual void StartNewRequest() override {
        // TODO
        return;
    }
    virtual void TakeAction(HandlerState *state) override {
        // TODO
        return; 
    }
};

class StatusHandler : public Handler<GRPCService::AsyncService, StatusRequest, StatusResponse> {
public:
    StatusHandler(GRPCService::AsyncService* svc, grpc::ServerCompletionQueue* cq, InferenceCore *inference_core) : Handler(svc,cq,inference_core) {}
    virtual ~StatusHandler() {}
    virtual void StartNewRequest() override {
        return ;
    }
    virtual void TakeAction(HandlerState *state) override {
        return ;
    }
};

class HealthHandler : public Handler<GRPCService::AsyncService, HealthRequest, HealthResponse> {
public:
    HealthHandler(GRPCService::AsyncService* svc, grpc::ServerCompletionQueue* cq, InferenceCore *inference_core) : Handler(svc,cq,inference_core) {}
    virtual ~HealthHandler() {}

    virtual void StartNewRequest() override {
        return; 
    }
    virtual void TakeAction(HandlerState *state) override {
        return; 
    }
};

Status
GrpcServer::Create(
    const std::shared_ptr<ServerConfig> server_config,
    InferenceCore* inference_core,
    std::unique_ptr<GrpcServer> *grpc_server) {
    LOG(INFO) << __FUNCTION__;
    std::unique_ptr<GrpcServer> grpc_server_local(new GrpcServer(server_config, inference_core));
    *grpc_server = std::move(grpc_server_local);
    return Status::Success;
}

GrpcServer::GrpcServer(const std::shared_ptr<ServerConfig> server_config,
    InferenceCore* inference_core) : server_cfg_(server_config), inference_core_(inference_core),running_(false) {
    LOG(INFO) << __FUNCTION__;
}

Status
GrpcServer::Start() {
    LOG(INFO) << "GRPCServer listen on " << server_cfg_->grpc_server_cfg_.grpc_server_addr_;
    grpc_builder_.AddListeningPort(server_cfg_->grpc_server_cfg_.grpc_server_addr_, grpc::InsecureServerCredentials());
    grpc_builder_.SetMaxMessageSize(constants::MAX_GRPC_MESSAGE_SIZE);
    grpc_builder_.RegisterService(&service_);

    infer_cq_ = grpc_builder_.AddCompletionQueue();
    status_cq_ = grpc_builder_.AddCompletionQueue();
    health_cq_ = grpc_builder_.AddCompletionQueue();

    grpc_server_ = grpc_builder_.BuildAndStart();

    infer_handler_.reset(new InferHandler(&service_, infer_cq_.get(), inference_core_));
    infer_handler_->Start();

    status_handler_.reset(new StatusHandler(&service_, status_cq_.get(), inference_core_));
    status_handler_->Start();

    health_handler_.reset(new HealthHandler(&service_, health_cq_.get(), inference_core_));
    health_handler_->Start();

    running_ = true;
    return Status::Success;
}

Status
GrpcServer::Stop() {
    LOG(INFO) << __FUNCTION__;

    if (!running_) {
        LOG(ERROR) << "GRPC server is already stopped.";
        return Status::Failed_Rpc_Not_Running;
    }
    grpc_server_->Shutdown();
    infer_cq_->Shutdown();
    status_cq_->Shutdown();
    health_cq_->Shutdown();

    dynamic_cast<InferHandler*>(infer_handler_.get())->Stop();
    dynamic_cast<StatusHandler*>(status_handler_.get())->Stop();
    dynamic_cast<HealthHandler*>(health_handler_.get())->Stop();
    running_ = false;

    return Status::Success;
}

GrpcServer::~GrpcServer() {
    if (running_) {
        Stop();
    }
    LOG(INFO) << "grpc server stopped.";
}

} // namespace model_inference_server