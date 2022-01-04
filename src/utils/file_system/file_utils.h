#pragma once

#include "file_system.h"

#include <string>
#include <vector>

namespace model_inference_server
{
namespace utils
{
class FileUtils{
public:
    FileUtils() = delete;
    static bool ExistPath(const std::string &path);
    static std::vector<std::string> GetDirectorySubDirs(const std::string &path);
    static std::string CombinePath(const std::string &base, const std::vector<std::string> &parts);
    static std::vector<std::string> GetDirectoryFiles(const std::string &path);
    static std::string ReadFileText(const std::string &path);
    static int64_t GetLastModifiedTime(const std::string &path);
    static bool DownloadFile(const std::string &path, std::string &local_path);

private:
    static FileSystem *GetFileSystem(const std::string &path);
};
} // namespace utils
} // namespace kwai_inference_engine
