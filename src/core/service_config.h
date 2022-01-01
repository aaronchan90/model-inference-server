#pragma once

#include "basic_data.h"
#include <memory>
#include <string>

namespace model_inference_server {

class ServiceHelper {
public:
    ServerHelper() = delete;

    static std::shared_ptr<ServerConfig> LoadServerConfig(const std::string &config_dir);

private:

    static void OverwriteFromCommandLine(std::shared_ptr<ServerConfig> &server_config);
}
}// model_inference_server