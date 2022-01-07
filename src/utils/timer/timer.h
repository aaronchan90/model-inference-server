#pragma once

#include <chrono>
#include <iomanip>

namespace model_inference_server
{
namespace utils
{
struct Timer {
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;

    std::chrono::time_point<std::chrono::high_resolution_clock> tic()
    {
        _start = std::chrono::high_resolution_clock::now();
        return _start;
    }

    int toc()
    {
        std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - _start;
        return int(diff.count() * 1000);  //ms
    }

    int toc(std::chrono::time_point<std::chrono::high_resolution_clock> start)
    {
        std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
        return int(diff.count() * 1000);  //ms
    }

    int toc(bool is_micro)
    {
        std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - _start;
        if (is_micro) {
            return int(diff.count() * 1000000);
        } else {
            return int(diff.count() * 1000);  //ms
        }
    }

    static double duration(
        std::chrono::time_point<std::chrono::high_resolution_clock> start,
        std::chrono::time_point<std::chrono::high_resolution_clock> end)
    {
        std::chrono::duration<double> diff = end - start;
        return diff.count() * 1000;  //ms
    }

    static int64_t duration_microsec(
        std::chrono::time_point<std::chrono::high_resolution_clock> start,
        std::chrono::time_point<std::chrono::high_resolution_clock> end)
    {
        std::chrono::duration<double> diff = end - start;
        return static_cast<int64_t>(diff.count() * 1000000);
    }
};
}
}  // namespace kwai_inference_engine