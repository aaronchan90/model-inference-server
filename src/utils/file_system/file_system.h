#pragma once

#include <string>
#include <vector>

namespace model_inference_server 
{
namespace utils
{
class FileSystem{
public:
    virtual bool ExistPath(const std::string &path) = 0;
    virtual std::vector<std::string> GetDirectorySubDirs(const std::string &path) = 0;
    virtual std::string CombinePath(const std::string &base, const std::vector<std::string> &parts) = 0;
    virtual std::vector<std::string> GetDirectoryFiles(const std::string &path) = 0;
    virtual std::string ReadFileText(const std::string &path) = 0;
    virtual int64_t GetLastModifiedTime(const std::string &path) = 0;
    virtual bool DownloadFile(const std::string &path, std::string &local_path) = 0;
};
} // namespace utils
} // namespace model_inference_server 