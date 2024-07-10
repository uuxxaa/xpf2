#include "Image.h"
#include <core/stringex.h>
#include <core/FileSystem.h>
// #include "texture.h"
// #include <glad/glad.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include <stb/stb_image_write.h>
#pragma clang diagnostic pop

#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#include <qoi.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg/nanosvg.h>
#include <nanosvg/nanosvgrast.h>
#pragma clang diagnostic pop

namespace xpf {

Image::Image(std::vector<byte_t>&& data, uint32_t width, uint32_t height, PixelFormat format, uint32_t mipMapCount)
    : m_data(std::move(data)), m_width(width), m_height(height), m_pixelFormat(format), m_mipMapCount(mipMapCount)
{}

Image::Image(const std::vector<byte_t>& data, uint32_t width, uint32_t height, PixelFormat format, uint32_t mipMapCount)
    : m_data(data), m_width(width), m_height(height), m_pixelFormat(format), m_mipMapCount(mipMapCount)
{}

/*static*/ Image Image::LoadImage(std::string_view filename)
{
    std::vector<byte_t> data = FileSystem::LoadFile(filename);
    if (data.empty())
        return Image();

    return LoadImage(filename, data);
}

/*static*/ Image Image::LoadImage(std::string_view filename, const std::vector<byte_t>& data)
{
    Image image;

    // callback gets the first dibs, then the built-in parsers
    if (s_imageParseCallback != nullptr)
        image = s_imageParseCallback(filename, data);

    if (image.IsEmpty())
        image = Image::CreateImage(filename, data);

    return image;
}

/*static*/ void Image::WriteDataCallback(void* pcontext, void* pdata, int size)
{
    std::vector<byte_t>& data = *((std::vector<byte_t>*)pcontext);
    data.insert(data.end(), (const byte_t*)pdata, (const byte_t*)pdata + size);
}

void Image::SaveImage(std::string_view filename, uint8_t quality)
{
    uint32_t channels = 4;
    if (m_pixelFormat == PixelFormat::GrayScale) channels = 1;
    else if (m_pixelFormat == PixelFormat::GrayAlpha) channels = 2;
    else if (m_pixelFormat == PixelFormat::R8G8B8) channels = 3;
    else if (m_pixelFormat == PixelFormat::R8G8B8A8) channels = 4;
    else throw std::runtime_error("unsupported pixel format for saving");

    xpf::stringex ext = stringex(filename.substr(filename.find_last_of(".") + 1)).to_lower();
    if (ext == "jpg")
    {
        //int dataSize = 0;
        std::vector<byte_t> data;
        data.reserve(m_width * m_height * channels / 2); // estimate 50% size reduction, callback is called multiple time, so it is better to reserve
        bool succeeded = stbi_write_jpg_to_func(Image::WriteDataCallback, &data, m_width, m_height, channels, m_data.data(), quality <= 100 ? quality : 100);
        if (succeeded)
            succeeded = FileSystem::SaveFile(filename, data.data(), data.size());

        if (!succeeded)
            throw std::runtime_error("failed to save to file");
    }
    else if (ext == "png")
    {
        std::vector<byte_t> data; // no need reserve as the callback for png is only called once
        bool succeeded = stbi_write_png_to_func(Image::WriteDataCallback, &data, m_width, m_height, channels, m_data.data(), 0);
        if (succeeded)
            succeeded = FileSystem::SaveFile(filename, data.data(), data.size());

        if (!succeeded)
            throw std::runtime_error("failed to save to file");
    }
    else if (ext == "bmp")
    {
        std::vector<byte_t> data;
        data.reserve(m_width * m_height * channels);
        bool succeeded = stbi_write_bmp_to_func(Image::WriteDataCallback, &data, m_width, m_height, channels, m_data.data());
        if (succeeded)
            succeeded = FileSystem::SaveFile(filename, data.data(), data.size());

        if (!succeeded)
            throw std::runtime_error("failed to save to file");
    }
    else
    {
        throw std::runtime_error("file format not supported");
    }
}

/*static*/ Image Image::CreateImage(std::string_view filename, const std::vector<byte_t>& dataIn)
{
    int compressionLevel = 0;
    int width = 0;
    int height = 0;
    PixelFormat format;

    stringex ext = xpf::stringex(filename.substr(filename.find_last_of(".") + 1)).to_lower();
    if (ext == "bmp" || ext == "gif" || ext == "jpg" || ext == "jpeg" ||
        ext == "png" || ext == "pic" || ext == "pgm" || ext == "psd" || ext == "tga")
    {
        const byte_t* pData = stbi_load_from_memory(
            dataIn.data(),
            static_cast<int>(dataIn.size()),
            &width,
            &height,
            &compressionLevel,
            /*require_compressed*/ 0);

        if (pData == nullptr)
            return Image();

        switch (compressionLevel)
        {
            case 1: format = PixelFormat::GrayScale; break;
            case 2: format = PixelFormat::GrayAlpha; break;
            case 3: format = PixelFormat::R8G8B8; break;
            case 4: format = PixelFormat::R8G8B8A8; break;
            default: stbi_image_free((void*)pData); return Image();
        }

        std::vector<byte_t> data(pData, pData + width * height * compressionLevel);
        stbi_image_free((void*)pData);

        return Image(std::move(data), width, height, format, /*mipMapCount:*/ 1);
    }
    else if (ext == "hdr") 
    {
        const byte_t* pData = (const byte_t*)stbi_loadf_from_memory(
            dataIn.data(),
            static_cast<int>(dataIn.size()),
            &width,
            &height,
            &compressionLevel,
            /*require_compressed*/ 0);

        if (pData == nullptr)
            return Image();

        switch (compressionLevel) {
            case 1: format = PixelFormat::R32; break;
            case 3: format = PixelFormat::R32G32B32; break;
            case 4: format = PixelFormat::R32G32B32A32; break;
            default: stbi_image_free((void*)pData); return Image();
        }

        std::vector<byte_t> data(pData, pData + width * height * compressionLevel * sizeof(float));
        stbi_image_free((void*)pData);
        return Image(std::move(data), width, height, format, /*mipMapCount:*/ 1);
    }
    else if (ext == "qoi")
    {
        qoi_desc desc = { 0, 0, 0, 0 };
        const byte_t* pData = (const byte_t*)qoi_decode(
            (const void*)dataIn.data(),
            static_cast<int>(dataIn.size()),
            &desc,
            /*channels:*/ 4); // 4 channels RGBA

        if (pData == nullptr)
            return Image();

        std::vector<byte_t> data(pData, pData + width * height * compressionLevel);
        stbi_image_free((void*)pData);
        return Image(std::move(data), width, height, PixelFormat::R8G8B8A8, /*mipMapCount:*/ 1);
    }
    else if (ext == "svg" &&
             dataIn.size() > 4 &&
             dataIn[0] == '<' &&  dataIn[1] == 's' &&  dataIn[2] == 'v' && dataIn[3] == 'g' && dataIn[3] == ' ')
    {
        // <svg ...
        NSVGimage* pSvg = nsvgParse((char*)(dataIn.data()), /*units:*/ "px", /*dpi:*/ 96.0f);
        std::vector<byte_t> data(pSvg->width * pSvg->height * 4);  // PixelFormat::R8G8B8A8 is 4 bytes

        NSVGrasterizer* pRasterizer = nsvgCreateRasterizer();
        nsvgRasterize(pRasterizer, pSvg,
            /*tx:*/0, /*ty:*/0, /*scale*/1.0f,
            data.data(), pSvg->width, pSvg->height, /*stride:*/ pSvg->width * 4);

        Image image(std::move(data), pSvg->width, pSvg->height, PixelFormat::R8G8B8A8, /*mipMapCount:*/ 1);
        nsvgDelete(pSvg);
        return image;
    }

    return Image();
}

#if 0
/*static*/ std::shared_ptr<xpf::texture> Image::CreateTexture(
    std::vector<byte_t>&& data,
    uint32_t width, uint32_t height,
    pixel_format format,
    uint32_t mipMapCount)
{
    Image img(std::move(data), width, height, format, mipMapCount);
    if (m_texture_factory != nullptr)
        return m_texture_factory(img);
    return img.load_texture();
}

std::tuple<uint32_t, uint32_t, uint32_t> Image::GetOpenGLTextureFormats() const
{
    switch (m_pixelFormat)
    {
        case PixelFormat::GrayScale: return { GL_R8, GL_RED, GL_UNSIGNED_BYTE };
        case PixelFormat::GrayAlpha: return { GL_RG8, GL_RG, GL_UNSIGNED_BYTE };
        case PixelFormat::R5G6B5: return { GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5 };
        case PixelFormat::R8G8B8: return { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE };
        case PixelFormat::R5G5B5A1: return { GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 };
        case PixelFormat::R4G4B4A4: return { GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 };
        case PixelFormat::R8G8B8A8: return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
        case PixelFormat::R32: return { GL_R32F, GL_RED, GL_FLOAT };
        case PixelFormat::R32G32B32: return { GL_RGB32F, GL_RGB, GL_FLOAT };
        case PixelFormat::R32G32B32A32: return { GL_RGBA32F, GL_RGBA, GL_FLOAT };
        case PixelFormat::R16: return { GL_R16F, GL_RED, GL_HALF_FLOAT };
        case PixelFormat::R16G16B16: return { GL_RGB16F, GL_RGB, GL_HALF_FLOAT };
        case PixelFormat::R16G16B16A16: return { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT };
        default: return { 0, 0, 0 };
    }
}
#endif

uint8_t Image::GetBytesPerPixel() const
{
    uint8_t bitsPerPixel;
    switch (m_pixelFormat)
    {
        case PixelFormat::GrayScale: bitsPerPixel = 8; break;
        case PixelFormat::AlphaOnly: bitsPerPixel = 8; break;
        case PixelFormat::GrayAlpha: bitsPerPixel = 16; break;
        case PixelFormat::R5G6B5: bitsPerPixel = 16; break;
        case PixelFormat::R5G5B5A1: bitsPerPixel = 16; break;
        case PixelFormat::R4G4B4A4: bitsPerPixel = 16; break;
        case PixelFormat::R8G8B8A8: bitsPerPixel = 32; break;
        case PixelFormat::R8G8B8: bitsPerPixel = 24; break;
        case PixelFormat::R32: bitsPerPixel = 32; break;
        case PixelFormat::R32G32B32: bitsPerPixel = 32 * 3; break;
        case PixelFormat::R32G32B32A32: bitsPerPixel = 32 * 4; break;
        case PixelFormat::R16: bitsPerPixel = 16; break;
        case PixelFormat::R16G16B16: bitsPerPixel = 16 * 3; break;
        case PixelFormat::R16G16B16A16: bitsPerPixel = 16 * 4; break;
        case PixelFormat::Compressed_DXT1_RGB: bitsPerPixel = 4; break;
        case PixelFormat::Compressed_DXT1_RGBA: bitsPerPixel = 4; break;
        case PixelFormat::Compressed_ETC1_RGB: bitsPerPixel = 4; break;
        case PixelFormat::Compressed_ETC2_RGB: bitsPerPixel = 4; break;
        case PixelFormat::Compressed_PVRT_RGB: bitsPerPixel = 4; break;
        case PixelFormat::Compressed_PVRT_RGBA: bitsPerPixel = 4; break;
        case PixelFormat::Compressed_DXT3_RGBA: bitsPerPixel = 8; break;
        case PixelFormat::Compressed_DXT5_RGBA: bitsPerPixel = 8; break;
        case PixelFormat::Compressed_ETC2_EAC_RGBA: bitsPerPixel = 8; break;
        case PixelFormat::Compressed_ASTC_4x4_RGBA: bitsPerPixel = 8; break;
        case PixelFormat::Compressed_ASTC_8x8_RGBA: bitsPerPixel = 2; break;
        default: bitsPerPixel = 0; break;
    }

    return bitsPerPixel / 8;
}

size_t Image::GetImageDataDize() const
{
    size_t dataSize = m_width * m_height * GetBytesPerPixel();  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if (m_width < 4 && m_height < 4)
    {
        if (((uint32_t)m_pixelFormat >= (uint32_t)PixelFormat::Compressed_DXT1_RGB) &&
            ((uint32_t)m_pixelFormat < (uint32_t)PixelFormat::Compressed_DXT3_RGBA))
        {
            dataSize = 8;
        }
        else if (
            ((uint32_t)m_pixelFormat >= (uint32_t)PixelFormat::Compressed_DXT3_RGBA) &&
            ((uint32_t)m_pixelFormat < (uint32_t)PixelFormat::Compressed_ASTC_8x8_RGBA))
        {
            dataSize = 16;
        }
    }

    return dataSize;
}

Image Image::SubImage(recti_t rect) const
{
    if (recti_t{0,0, int32_t(m_width), int32_t(m_height)}.intersection(rect).is_empty())
        return Image(); // empty

    if (m_pixelFormat >= PixelFormat::Compressed_DXT1_RGB)
        return Image(); // not supported

    if (m_mipMapCount != 1)
        return Image(); // not supported

    const uint8_t bytesPerPixel = GetBytesPerPixel();

    std::vector<byte_t> target(rect.width * rect.height * bytesPerPixel);

    for (int32_t i = 0; i < rect.height; i++)
    {
        memcpy(
            target.data() + i * rect.width * bytesPerPixel,
            m_data.data() + ((i + rect.y) * m_width + rect.x) * bytesPerPixel,
            rect.width * bytesPerPixel);
    }

    return Image(
        std::move(target),
        rect.width,
        rect.height,
        m_pixelFormat,
        m_mipMapCount);
}

Image& Image::VerticalFlip()
{
    stbi__vertical_flip(m_data.data(), m_width, m_height, GetBytesPerPixel());
    return *this;
}

Image Image::Resize(uint32_t width, uint32_t height) {
    std::vector<byte_t> data;
    data.resize(width * height * 4);
    stbir_resize_uint8(m_data.data(), m_width, m_height, 0, data.data(), width, height, 0, 4);
    return Image(std::move(data), width, height, PixelFormat::R8G8B8A8);
}

} // xpf