#include <cstring>
#include "file_utils.h"
#include "local_file_system.h"

using namespace model_inference_server;
using namespace model_inference_server::utils;

FileSystem* 
FileUtils::GetFileSystem(const std::string &path){
    static LocalFileSystem local_file_system;
    return &local_file_system;
}

bool 
FileUtils::ExistPath(const std::string &path){
    const auto fs = GetFileSystem(path);
    return fs->ExistPath(path);
}

std::vector<std::string> 
FileUtils::GetDirectorySubDirs(const std::string &path){
    const auto fs = GetFileSystem(path);
    return fs->GetDirectorySubDirs(path);
}

std::string 
FileUtils::CombinePath(const std::string &base, const std::vector<std::string> &parts){
    const auto fs = GetFileSystem(base);
    return fs->CombinePath(base, parts);
}
std::string 
FileUtils::ReadFileText(const std::string &path){
    const auto fs = GetFileSystem(path);
    return fs->ReadFileText(path);
}

std::vector<std::string> 
FileUtils::GetDirectoryFiles(const std::string &path){
    const auto fs = GetFileSystem(path);
    return fs->GetDirectoryFiles(path);
}

int64_t 
FileUtils::GetLastModifiedTime(const std::string &path){
    const auto fs = GetFileSystem(path);
    return fs->GetLastModifiedTime(path);
}

bool 
FileUtils::DownloadFile(const std::string &path, std::string &local_path){
    const auto fs = GetFileSystem(path);
    return fs->DownloadFile(path, local_path);
}
