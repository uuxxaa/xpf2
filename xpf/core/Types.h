#pragma once
#include <stdint.h>
#include <memory>

typedef uint8_t byte_t;
typedef uint32_t textureid_t;

namespace xpf {

template<typename T>
struct Array
{
    std::shared_ptr<T[]> spData;
    uint32_t length = 0;

    const T* Data() const { return spData.get(); }
    uint32_t GetSizeInBytes() const { return length * sizeof(T); }
};

} // xpf