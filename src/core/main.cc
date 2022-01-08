#include <gflags/gflags.h>
#include <glog/logging.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <csignal>

#include "server/inference_core.h"
#include "service_config.h"

using namespace model_inference_server;

DEFINE_string(config, "", "service config-file path");
DEFINE_int32(exit_wait, 120, "wait time before exit");

volatile ExitSignal exiting_ = NoSignal;
std::mutex exit_mu_;
std::condition_variable exit_cv_;

void pipeSignalHandler(int signum) {
    LOG_FIRST_N(ERROR, 10) << "pipeSignalHandler: " << signum;
}

void signalHandler(int signum) {
    LOG_FIRST_N(ERROR, 10) << "signalHandler: " << signum;
    if (exiting_ != NoSignal) {
        return ;
    }
    {
        std::unique_lock<std::mutex> lock(exit_mu_);
        if (signum == SIGTERM) {
            exiting_ = GraceExit;
        }else{
            exiting_ = ImmediateExit;
        }
    }
    exit_cv_.notify_all();
}

int main(int argc, char *argv[]) {

    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    
    // catch exit signals
    signal(SIGPIPE, pipeSignalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // decode server configures
    std::shared_ptr<ServerConfig> service_config = ServiceHelper::LoadServerConfig(FLAGS_config);

    LOG(INFO) << service_config;
    // create inference core object

    LOG(INFO) << "service starting...";
    InferenceCore service_core(service_config);
    if (service_core.Start() != Status::Success) {
        LOG(ERROR) << "Failed to start inference core service.";
        return -1;
    }
    
    // wait for exit signals and wait enought time for grace exit
    {
        std::unique_lock<std::mutex> lock(exit_mu_);
        exiting_ = NoSignal;
    }
    while(exiting_ == NoSignal) {
        std::unique_lock<std::mutex> lock(exit_mu_);
        std::chrono::seconds wait_timeout(3600);
        exit_cv_.wait_for(lock, wait_timeout);
        LOG(INFO) << "main thread wait up, exiting_: " << exiting_;
    }
    if (exiting_ == GraceExit) {
        LOG(INFO) << "sleep for " << FLAGS_exit_wait << " seconds";
        std::chrono::seconds wait_time(FLAGS_exit_wait);
        std::this_thread::sleep_for(wait_time);
    }
    if (service_core.Stop() != Status::Success) {
        LOG(ERROR) << "Failed to stop inference core service.";
        return -1;
    }
    LOG(INFO) << "service exit";

    return 0;
}