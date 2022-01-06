#pragma once

#include <cstdint>

namespace model_inference_server 
{
class MemoryReference {
public:
    MemoryReference(const uint8_t *mem_addr, uint64_t length) : mem_addr_(mem_addr), length_(length) {}
    const uint8_t *MemoryAddress() { return mem_addr_; }
    uint64_t Length() { return length_; }
private:
    const uint8_t *mem_addr_ = nullptr;
    uint64_t length_ = 0;
};
} // namespace model_inference_server 