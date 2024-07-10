#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import "Metal.hpp"

#include <string>
#include <memory>
#include <simd/simd.h>

#include <common/Common_Renderer.h>
#include <core/Color.h>
#include <core/Convert_Apple.h>
#include <core/Image.h>
#include <core/Log.h>
#include <renderer/ITexture.h>

namespace xpf {

class MetalTexture : public ITexture
{
public:
    id<MTLTexture> m_texture;
    const textureid_t m_textureid;
    const uint32_t m_width;
    const uint32_t m_height;
    rectf_t m_textCoords;
    const ITexture::Interpolation m_interpolation;
    static inline textureid_t s_id = 0;

public:
    MetalTexture(id<MTLTexture> texture, uint32_t width, uint32_t height, Interpolation interpolation, rectf_t region)
        : m_texture(texture)
        , m_textureid(+s_id)
        , m_width(width)
        , m_height(height)
        , m_textCoords(region)
        , m_interpolation(interpolation)
         {}

    virtual textureid_t GetId() const override { return m_textureid; }
    virtual uint32_t GenerateMipMaps() override { return 1; }
    virtual uint32_t GetWidth() const override { return m_width; }
    virtual uint32_t GetHeight() const override { return m_height; }
    virtual rectf_t GetRegion() const override { return m_textCoords; }
    virtual ITexture::Interpolation GetInterpolation() const override { return m_interpolation; }

    virtual std::shared_ptr<ITexture> SampledTexture(Interpolation interpolation, rectf_t region) const override
    {
        return std::make_shared<MetalTexture>(m_texture, m_width, m_height, interpolation, region);
    }

    virtual void SetRegion(rectf_t region) override { m_textCoords = region; }
};

id<MTLTexture> GetMTLTexture(const ITexture* pTexture)
{
    return static_cast<const MetalTexture*>(pTexture)->m_texture;
}

std::shared_ptr<ITexture> Metal_CreateTexture(id<MTLDevice> device, const Image& image)
{
    // Indicate that each pixel has a blue, green, red, and alpha channel, where each channel is
    // an 8-bit unsigned normalized value (i.e. 0 maps to 0.0 and 255 maps to 1.0)
    uint32_t bytesPerPixel = 4;
    MTLPixelFormat pixelFormat;
    switch (image.GetPixelFormat())
    {
        case PixelFormat::R8G8B8A8:
            pixelFormat = MTLPixelFormatRGBA8Unorm;
            bytesPerPixel = 4;
            break;
        case PixelFormat::GrayScale:
            pixelFormat = MTLPixelFormatR8Unorm;
            bytesPerPixel = 1;
            break;
        case PixelFormat::R4G4B4A4:
            // https://github.com/KhronosGroup/MoltenVK/issues/353
            pixelFormat = MTLPixelFormatABGR4Unorm;
            bytesPerPixel = 2;
            break;
        default:
            return nullptr;
    }

    MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat: pixelFormat
                                     width: image.GetWidth()
                                    height: image.GetHeight()
                                 mipmapped: NO];

    // Calculate the number of bytes per row in the image.
    NSUInteger bytesPerRow = bytesPerPixel * image.GetWidth();
    MTLRegion region = MTLRegionMake2D(0,0, image.GetWidth(), image.GetHeight());

    // Create the texture from the device by using the descriptor
    id<MTLTexture> texture = [device newTextureWithDescriptor: textureDescriptor];

    // Copy the bytes from the data object into the texture
    [texture  replaceRegion: region
                mipmapLevel: 0
                  withBytes: image.GetData().data()
                bytesPerRow: bytesPerRow];

    return std::make_shared<MetalTexture>(
        texture,
        image.GetWidth(), image.GetHeight(),
        ITexture::Interpolation::None,
        rectf_t{0,0,1,1});
}

} // xpf