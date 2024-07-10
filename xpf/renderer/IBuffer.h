#pragma once
#include <core/Types.h>
#include <functional>
#include <memory>

namespace xpf {

class IBuffer
{
public:
    virtual ~IBuffer() = default;

    virtual uint32_t GetId() const = 0;
    virtual size_t GetSize() const = 0;

    static inline std::function<
        std::shared_ptr<IBuffer>(
            const byte_t* pdata, size_t size
            )> BufferLoader;
};

} // xpf