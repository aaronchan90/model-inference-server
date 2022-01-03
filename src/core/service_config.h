#pragma once

#include <memory>
#include <string>
#include <iostream>

#include "basic_data.h"

namespace model_inference_server {

class ServiceHelper {
public:
    ServiceHelper() = delete;
    static std::shared_ptr<ServerConfig> LoadServerConfig(const std::string &config_dir);

    friend std::ostream &operator<<(std::ostream &out, const ServerConfig &server_config);
    
private:
    static void OverwriteFromCommandLine(std::shared_ptr<ServerConfig> &server_config);
};
}// model_inference_server