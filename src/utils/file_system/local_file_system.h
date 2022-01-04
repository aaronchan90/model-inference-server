#pragma once

#include <string>
#include <vector>

#include "file_system.h"

namespace model_inference_server 
{
namespace utils
{
class LocalFileSystem : public FileSystem{
public:
    virtual bool ExistPath(const std::string &path) override;
    virtual std::vector<std::string> GetDirectorySubDirs(const std::string &path) override;
    virtual std::string CombinePath(const std::string &base, const std::vector<std::string> &parts) override;
    virtual std::vector<std::string> GetDirectoryFiles(const std::string &path) override;
    virtual std::string ReadFileText(const std::string &path) override;
    virtual int64_t GetLastModifiedTime(const std::string &path) override;
    virtual bool DownloadFile(const std::string &path, std::string &local_path) override;
};
} // namespace utils
} // namespace model_inference_server 
