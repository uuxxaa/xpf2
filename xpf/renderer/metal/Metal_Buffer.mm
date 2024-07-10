#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <renderer/IBuffer.h>

namespace xpf {

class MetalBuffer : public IBuffer
{
public:
    id<MTLBuffer> m_buffer;
    const uint32_t m_id;
    const size_t m_size;
    static inline uint32_t s_id = 0;

public:
    MetalBuffer(id<MTLBuffer> texture, size_t size)
        : m_buffer(texture)
        , m_id(+s_id)
        , m_size(size)
         {}

    ~MetalBuffer() {
        return;
    }

    virtual uint32_t GetId() const override { return m_id; }
    virtual size_t GetSize() const override { return m_size; }
};

id<MTLBuffer> GetMTLBuffer(const IBuffer* pBuffer)
{
    return static_cast<const MetalBuffer*>(pBuffer)->m_buffer;
}

std::shared_ptr<IBuffer> Metal_CreateBuffer(id<MTLDevice> device, const byte_t* pbyte, size_t size)
{
    id<MTLBuffer> mtlBuffer = [device
        newBufferWithBytes: pbyte
                    length: (NSUInteger)size
                   options: MTLResourceStorageModeShared];
    return std::make_shared<MetalBuffer>(mtlBuffer, size);
}

} // xpf