#include "Common_Renderer.h"
#include <renderer/IBuffer.h>
#include <renderer/ITexture.h>
#include <renderer/common/Font.h>
#include <core/Image.h>
#include <core/Quad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace xpf {

GLFWwindow* CommonRenderer::Initialize(RendererOptions&& optionsIn)
{
    m_options = std::move(optionsIn);

    m_background_color = m_options.background_color.get_vec4();
    m_foreground_color = m_options.foreground_color.get_vec4();

    if (m_options.is_projection_ortho2d)
    {
        m_projection_matrix = m4_t::ortho(
            /*left:*/0.0f, /*right:*/float(m_options.width),
            /*bottom:*/float(m_options.height), /*top:*/0.0f);
    }

    m_pWindow = glfwCreateWindow(m_options.width, m_options.height, m_options.title.c_str(), nullptr, nullptr);
    if (m_pWindow == nullptr)
    {
        glfwTerminate();
        return nullptr;
    }

    CommonRenderer* pRenderer = this;
    xpf::ITexture::TextureLoader = [pRenderer](
        std::vector<byte_t>&& data,
        uint32_t width, uint32_t height,
        PixelFormat pixelFormat,
        uint16_t mipMapCount)
    {
        Image img(std::move(data), width, height, pixelFormat, mipMapCount);
        return pRenderer->CreateTexture(img);
    };

    xpf::IBuffer::BufferLoader = [pRenderer](const byte_t* pbyte, size_t size)
    {
        return pRenderer->CreateBuffer(pbyte, size);
    };

    return m_pWindow;
}

void CommonRenderer::CaptureScreen(
    uint32_t frameCount,
    std::function<void(Image&&)>&& fn)
{
    CaptureScreen(frameCount, {0, 0, int32_t(m_options.width), int32_t(m_options.height)}, std::move(fn));
}

void CommonRenderer::CaptureScreen(
    uint32_t frameCount,
    recti_t region,
    std::function<void(Image&&)>&& fn)
{
    m_captureScreen_frameCount = frameCount;
    m_captureScreen_region = region;
    m_captureScreen_callback = std::move(fn);
}

void CommonRenderer::EnqueueCommands(const RenderBatch& batch)
{
    m_builder.Flush();

    for (const auto& spCmd : batch.commands)
        m_builder.AppendCommand(spCmd);
}

void CommonRenderer::OnResize(int32_t width, int32_t height)
{
    m_options.width = width;
    m_options.height = height;
    m_projection_matrix = m4_t::ortho(
        /*left:*/0.0f, /*right:*/float(m_options.width),
        /*bottom:*/float(m_options.height), /*top:*/0.0f);
}

RenderBatchBuilder CommonRenderer::CreateCommandBuilder()
{
    return RenderBatchBuilder(this);
}

void CommonRenderer::Flush()
{
    m_builder.Flush();
}

void CommonRenderer::RenderScope(std::function<void()>&& fn)
{
    fn();
}

RenderStats CommonRenderer::GetStats()
{
    return m_renderStats;
}

RendererCapability CommonRenderer::GetCapabilities() const
{
    return m_options.capabilities;
}

#pragma region
StackGuard CommonRenderer::Transform(const m4_t& transform, bool multiply)
{
    return m_builder.Transform(transform, multiply);
}

StackGuard CommonRenderer::TranslateTransfrom(float x, float y)
{
    return m_builder.TranslateTransfrom(x, y);
}

StackGuard CommonRenderer::RotateTransform(radians_t angle)
{
    return m_builder.RotateTransform(angle);
}

const m4_t& CommonRenderer::GetCurrentTransform() const
{
    return m_builder.GetCurrentTransform();
}

StackGuard CommonRenderer::Clip(rectui_t region)
{
    return m_builder.Clip(region);
}

void CommonRenderer::DrawBezierCubic(v2_t p1, v2_t p2, v2_t p3, v2_t p4, float width, xpf::Color color, LineOptions lineOptions, uint32_t detail)
{
    m_builder.DrawBezierCubic(p1, p2, p3, p4, width, color, lineOptions, detail);
}

void CommonRenderer::DrawBezierCubic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, const PolyLineVertex& v4, LineOptions lineOptions, uint32_t detail)
{
    m_builder.DrawBezierCubic(v1, v2, v3, v4, lineOptions, detail);
}

void CommonRenderer::DrawBezierQuadratic(v2_t p1, v2_t p2, v2_t p3, float width, xpf::Color color, LineOptions lineOptions, uint32_t detail)
{
    m_builder.DrawBezierQuadratic(p1, p2, p3, width, color, lineOptions, detail);
}

void CommonRenderer::DrawBezierQuadratic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, LineOptions lineOptions, uint32_t detail)
{
    m_builder.DrawBezierQuadratic(v1, v2, v3, lineOptions, detail);
}

void CommonRenderer::DrawCircle(float x, float y, float radius, xpf::Color color)
{
    m_builder.DrawCircle(x, y, radius, color);
}

void CommonRenderer::DrawCircle(float x, float y, const CircleDescription& description)
{
    m_builder.DrawCircle(x, y, description);
}

void CommonRenderer::DrawImage(float x, float y, float w, float h, const std::shared_ptr<ITexture>& spTexture, const rectf_t& coords, xpf::Color color)
{
    m_builder.DrawImage(x, y, w, h, spTexture, coords, color);
}

void CommonRenderer::DrawLine(float x0, float y0, float x1, float y1, float width, xpf::Color color, LineOptions lineOptions)
{
    m_builder.DrawLine(x0, y0, x1, y1, width, color, lineOptions);
}

void CommonRenderer::DrawLine(const std::vector<PolyLineVertex>& points, LineOptions lineOptions)
{
    m_builder.DrawLine(points, lineOptions);
}

void CommonRenderer::DrawRectangle(rectf_t rect, xpf::Color color)
{
    m_builder.DrawRectangle(rect, color);
}

void CommonRenderer::DrawRectangle(float x, float y, float w, float h, xpf::Color color)
{
    m_builder.DrawRectangle(x, y, w, h, color);
}

void CommonRenderer::DrawRectangle(float x, float y, const RectangleDescription& description)
{
    m_builder.DrawRectangle(x, y, description);
}

void CommonRenderer::DrawText(std::string_view text, float x, float y, std::string_view fontName, uint16_t fontSize, xpf::Color color)
{
    m_builder.DrawText(text, x, y, fontName, fontSize, color);
}

rectf_t CommonRenderer::DrawText(std::string_view text, float x, float y, const TextDescription& description)
{
    return m_builder.DrawText(text, x, y, description);
}

void CommonRenderer::DrawText(float x, float y, FormattedText& ft, xpf::Color color)
{
    return m_builder.DrawText(x, y, ft, color);
}
#pragma endregion

} // xpf
