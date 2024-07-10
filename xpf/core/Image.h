#pragma once
#include <core/Rectangle.h>
#include <core/Types.h>
#include <functional>
#include <vector>
#undef LoadImage

namespace xpf {

enum class PixelFormat {
    Undefined = 0,
    GrayScale = 1,              // 8 bit per pixel (no alpha)
    AlphaOnly,
    GrayAlpha,                  // 8*2 bpp (2 channels)
    R5G6B5,                     // 16 bpp
    R8G8B8,                     // 24 bpp
    R5G5B5A1,                   // 16 bpp (1 bit alpha)
    R4G4B4A4,                   // 16 bpp (4 bit alpha)
    R8G8B8A8,                   // 32 bpp
    R32,                        // 32 bpp (1 channel - float)
    R32G32B32,                  // 32*3 bpp (3 channels - float)
    R32G32B32A32,               // 32*4 bpp (4 channels - float)
    R16,                        // 16 bpp (1 channel - half float)
    R16G16B16,                  // 16*3 bpp (3 channels - half float)
    R16G16B16A16,               // 16*4 bpp (4 channels - half float)
    Compressed_DXT1_RGB,        // 4 bpp (no alpha)
    Compressed_DXT1_RGBA,       // 4 bpp (1 bit alpha)
    Compressed_DXT3_RGBA,       // 8 bpp
    Compressed_DXT5_RGBA,       // 8 bpp
    Compressed_ETC1_RGB,        // 4 bpp
    Compressed_ETC2_RGB,        // 4 bpp
    Compressed_ETC2_EAC_RGBA,   // 8 bpp
    Compressed_PVRT_RGB,        // 4 bpp
    Compressed_PVRT_RGBA,       // 4 bpp
    Compressed_ASTC_4x4_RGBA,   // 8 bpp
    Compressed_ASTC_8x8_RGBA,   // 2 bpp
};

class Image
{
protected:
    std::vector<byte_t> m_data;     // raw image data
    uint32_t m_width = 0;           // width of the image
    uint32_t m_height = 0;          // height of the image
    PixelFormat m_pixelFormat = PixelFormat::Undefined;
    uint32_t m_mipMapCount = 1;     // mipmap count - https://en.wikipedia.org/wiki/Mipmap

    static inline std::function<Image(std::string_view, const std::vector<byte_t>&)> s_imageParseCallback;

public:
    Image() = default;
    Image(Image&& other) = default;
    Image(const Image& other) = delete; // to prevent accidental copying
    Image(std::vector<byte_t>&& data, uint32_t width, uint32_t height, PixelFormat format, uint32_t mipMapCount = 1);
    Image(const std::vector<byte_t>& data, uint32_t width, uint32_t height, PixelFormat format, uint32_t mipMapCount = 1);

    static Image LoadImage(std::string_view filename);
    static Image LoadImage(std::string_view filename, const std::vector<byte_t>& data);
    void SaveImage(std::string_view filename, uint8_t quality = 100);

    Image& operator=(Image&& other) = default;
    Image& operator=(const Image& other) = delete; // to prevent accidental copying

    bool IsEmpty() const { return m_data.empty(); }

    const std::vector<byte_t> GetData() const { return m_data; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetMipMapCount() const { return m_mipMapCount; }
    PixelFormat GetPixelFormat() const { return m_pixelFormat; }

    Image SubImage(recti_t rect) const;
    Image& VerticalFlip();

    Image Resize(uint32_t width, uint32_t height);

protected:
    static Image CreateImage(std::string_view filename, const std::vector<byte_t>& data);
    uint8_t GetBytesPerPixel() const;
    size_t GetImageDataDize() const;
    std::tuple<uint32_t, uint32_t, uint32_t> GetOpenGLTextureFormats() const;
    static void WriteDataCallback(void* pcontext, void* pdata, int size);
};

} // xpf