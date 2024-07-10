#include <renderer/ITexture.h>
#include <core/Image.h>
#include <core/Types.h>

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace xpf {

class OpenGLTexture : public ITexture
{
protected:
    Type                   m_type = Type::Texture2D;
    const textureid_t      m_id = 0;          // texture id - managed by OpenGl & tracked by OpenGLTexture
    const uint32_t         m_width = 0;       // width of the image
    const uint32_t         m_height = 0;      // height of the image
    const uint32_t         m_mipMapCount = 1; // mipmap count - https://en.wikipedia.org/wiki/Mipmap
    const xpf::PixelFormat m_pixelFormat = PixelFormat::Undefined;  // Data format (pixel_format type)
    rectf_t                m_textCoords;
    const Interpolation    m_interpolation;

public:
    OpenGLTexture(uint32_t id, uint32_t width, uint32_t height, uint32_t mipMapCount, PixelFormat format, Interpolation interpolation, rectf_t region)
        : m_id(id), m_width(width), m_height(height)
        , m_mipMapCount(mipMapCount), m_pixelFormat(format)
        , m_textCoords(region)
        , m_interpolation(interpolation)
    {}

    ~OpenGLTexture()
    {
        if (m_id != 0)
            glDeleteTextures(1, &m_id);
    }

    virtual textureid_t GetId() const override { return m_id; }
    virtual uint32_t GetWidth() const override { return m_width; }
    virtual uint32_t GetHeight() const override { return m_height; }
    virtual rectf_t GetRegion() const override { return m_textCoords; }
    virtual Interpolation GetInterpolation() const override { return m_interpolation; }
    virtual std::shared_ptr<ITexture> SampledTexture(Interpolation interpolation, rectf_t region) const override
    {
        return std::make_shared<xpf::OpenGLTexture>(m_id, m_width, m_height, m_mipMapCount, m_pixelFormat, interpolation, region);
    }

    virtual void SetRegion(rectf_t region) override { m_textCoords = region; }

    Type GetType() const { return m_type; }
    uint32_t GetMipMapCount() const { return m_mipMapCount; }
    PixelFormat GetPixelFormat() const { return m_pixelFormat; }

    virtual uint32_t GenerateMipMaps() override
    {
        uint32_t mipmaps = 0;
        glBindTexture(GL_TEXTURE_2D, m_id);

        const bool isTexturePowerOfTwo =
            (((m_width > 0) && ((m_width & (m_width - 1)) == 0)) &&
            ((m_height > 0) && ((m_height & (m_height - 1)) == 0)));

        if (isTexturePowerOfTwo)
        {
            glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);   // Hint for mipmaps generation algorithm: GL_FASTEST, GL_NICEST, GL_DONT_CARE
            glGenerateMipmap(GL_TEXTURE_2D);    // Generate mipmaps automatically

            mipmaps = 1 + (uint32_t)std::floorf(std::logf(std::max(m_width, m_height)) / std::logf(2));
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        return mipmaps;
    }
};

static std::tuple<uint32_t, uint32_t, uint32_t> GetOpenGLTextureFormats(PixelFormat pixelFormat)
{
    switch (pixelFormat)
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

std::shared_ptr<ITexture> OpenGL_CreateTexture(const Image& img)
{
    const std::vector<byte_t>& imgdata = img.GetData();
    const uint32_t imgWidth = img.GetWidth();
    const uint32_t imgHeight = img.GetHeight();
    const PixelFormat pixelFormat = img.GetPixelFormat();
    const uint32_t mipMapCount = img.GetMipMapCount();
    const int32_t imageSize = static_cast<int32_t>(imgdata.size());
    if (imageSize == 0)
        return nullptr;

    uint32_t id = 0;
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    uint32_t mipWidth = imgWidth;
    uint32_t mipHeight = imgHeight;
    uint32_t mipOffset = 0; // Mipmap data offset

    const auto [glInternalFormat, glFormat, glType] = GetOpenGLTextureFormats(pixelFormat);
    if (glInternalFormat == 0)
        return nullptr;

    // Load the different mipmap levels
    for (uint32_t i = 0; i < mipMapCount; i++)
    {
        if (pixelFormat < PixelFormat::Compressed_DXT1_RGB)
            glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, imgdata.data() + mipOffset);
        else
            glCompressedTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, imageSize, imgdata.data() + mipOffset);

        if (pixelFormat == PixelFormat::GrayScale)
        {
            GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }
        else if (pixelFormat == PixelFormat::AlphaOnly)
        {
            GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }
        else if (pixelFormat == PixelFormat::GrayAlpha)
        {
            GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }

        mipWidth /= 2;
        mipHeight /= 2;
        mipOffset += imageSize;

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis

    // Magnification and minification filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR

    if (img.GetMipMapCount() > 1)
    {
        // Activate Trilinear filtering if mipmaps are available
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    // At this point we have the texture loaded in GPU and texture parameters configured
    // NOTE: If mipmaps were not in data, they are not generated automatically

    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return std::make_shared<xpf::OpenGLTexture>(
        id,
        imgWidth, imgHeight, mipMapCount, pixelFormat,
        ITexture::Interpolation::None, rectf_t{0,0,1,1});
}

} // xpf
