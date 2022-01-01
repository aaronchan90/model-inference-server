#include <gflags/gflags.h>
#include <glog/logging.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "service_config.h"

DEFINE_string(config, "default", "service config-file path");

using namespace model_inference_server;

void pipeSignalHandler(int signum) {
    LOG_FIRST_N(ERROR, 10) << "pipeSignalHandler: " << signum;
    // TODO
}

void signalHandler(int signum) {
    LOG_FIRST_N(ERROR, 10) << "signalHandler: " << signum;
    // TODO
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
    //auto service_config = ServiceHelper::LoadServerConfig(FLAGS_config);

    // create inference-server object

    LOG(INFO) << "service starting...";
    
    // TODO
    // wait for exit signals and wait enought time for grace exit

    LOG(INFO) << "service exit";

    return 0;
}