#pragma once
#include <core/Rectangle.h>
#include <core/Types.h>
#include <functional>
#include <memory>

namespace xpf {

enum class PixelFormat;

class ITexture
{
public:
    enum class Type
    {
        Texture2D,
        Texture3D,
    };

    enum class Interpolation
    {
        None,
        Linear,
    };

    virtual ~ITexture() = default;

    virtual textureid_t GetId() const = 0;
    virtual uint32_t GenerateMipMaps() = 0;
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual rectf_t GetRegion() const = 0;
    virtual Interpolation GetInterpolation() const = 0;
    virtual std::shared_ptr<ITexture> SampledTexture(Interpolation interpolation, rectf_t region = {0,0,1,1}) const = 0;
    virtual void SetRegion(rectf_t region) = 0;

    static inline std::function<
        std::shared_ptr<ITexture>(
            std::vector<byte_t>&& data,
            uint32_t width,
            uint32_t height,
            PixelFormat format,
            uint16_t mipMapCount)> TextureLoader;
};

} // xpf