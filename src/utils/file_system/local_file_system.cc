#include "local_file_system.h"

#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <glog/logging.h>

using namespace model_inference_server::utils;

bool 
LocalFileSystem::ExistPath(const std::string &path){
    return boost::filesystem::exists(path);
}

std::vector<std::string> 
LocalFileSystem::GetDirectorySubDirs(const std::string &path){
    std::vector<std::string> dirs;
    for (auto &p : boost::filesystem::directory_iterator(path)){
        if (boost::filesystem::is_directory(p)){
            dirs.push_back(p.path().filename().string());
        }
    }
    return dirs;
}

std::string 
LocalFileSystem::CombinePath(const std::string &base, const std::vector<std::string> &parts){
    auto p = boost::filesystem::path(base);
    for (const auto &part : parts){
        p = p / part;
    }
    return p.string();
}

std::vector<std::string> 
LocalFileSystem::GetDirectoryFiles(const std::string &path){
    std::vector<std::string> dirs;
    for (auto &p : boost::filesystem::directory_iterator(path)){
        if (boost::filesystem::is_regular_file(p)){
            dirs.push_back(p.path().filename().string());
        }
    }
    return dirs;
}

std::string 
LocalFileSystem::ReadFileText(const std::string &path){
    std::ifstream in_file(path);
    if (in_file.is_open()){
        std::stringstream ss;
        ss << in_file.rdbuf();
        return ss.str();
    }
    return {};
}

int64_t 
LocalFileSystem::GetLastModifiedTime(const std::string &path){
    if (boost::filesystem::exists(path)){
        std::time_t t = boost::filesystem::last_write_time(path);
        return static_cast<int64_t>(t);
    }else{
        return -1;
    }
}

bool 
LocalFileSystem::DownloadFile(const std::string &path, std::string &local_path){
    local_path = path;
    return true;
}
