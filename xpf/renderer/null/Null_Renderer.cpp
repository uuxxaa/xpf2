#include <common/Common_Renderer.h>
#include <renderer/IBuffer.h>
#include <renderer/ITexture.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace xpf {

class NullTexture : public ITexture
{
public:
    NullTexture() = default;
    virtual ~NullTexture() override = default;
    virtual textureid_t GetId() const override { return 1; }
    virtual uint32_t GenerateMipMaps() override { return 1; }
    virtual uint32_t GetWidth() const override { return 1; }
    virtual uint32_t GetHeight() const override { return 1; }
    virtual rectf_t GetRegion() const override { return {0,0,1,1}; }
    virtual Interpolation GetInterpolation() const override { return Interpolation::None; }
    virtual std::shared_ptr<ITexture> SampledTexture(Interpolation /*interpolation*/, rectf_t /*region*/) const override { return nullptr; }
    virtual void SetRegion(rectf_t /*region*/) override { };
};

class NullBuffer : public IBuffer
{
public:
    NullBuffer() = default;
    virtual ~NullBuffer() override = default;
    virtual uint32_t GetId() const override { return 1; }
    virtual size_t GetSize() const override { return 1; }
};

class NullRenderer : public CommonRenderer
{
public:
    NullRenderer() = default;

    virtual GLFWwindow* Initialize(RendererOptions&& optionsIn) override
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if (CommonRenderer::Initialize(std::move(optionsIn)) == nullptr)
            return nullptr;

        return m_pWindow;
    }

    virtual void Shutdown() override { }
    virtual void Render() override { m_frame_count++; }

    // Builder methods
    virtual std::shared_ptr<ITexture> CreateTexture(std::string_view /*filename*/) override
    {
        return std::make_shared<NullTexture>();
    }

    virtual std::shared_ptr<ITexture> CreateTexture(const Image& /*img*/) override
    {
        return std::make_shared<NullTexture>();
    }

    virtual std::shared_ptr<IBuffer> CreateBuffer(const byte_t* /*pdata*/, size_t /*size*/) override
    {
        return std::make_shared<NullBuffer>();
    }
};

std::unique_ptr<IRenderer> create_null_renderer()
{
    return std::make_unique<NullRenderer>();
}

} // xpf