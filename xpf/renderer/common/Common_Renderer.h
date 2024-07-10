#pragma once
#include <core/Image.h>
#include <core/Types.h>
#include <renderer/IRenderer.h>
#include <renderer/common/RenderBatchBuilder.h>
#include <math/m4_t.h>
#include <math/v4_t.h>
#include <vector>

namespace xpf {

class CommonRenderer : public IRenderer
{
protected:
    RendererOptions m_options;
    m4_t m_projection_matrix = m4_t::identity;
    GLFWwindow* m_pWindow = nullptr;
    v4_t m_background_color;
    v4_t m_foreground_color;
    RenderBatchBuilder m_builder;
    uint64_t m_frame_count = 0;
    RenderStats m_renderStats;

    uint32_t m_captureScreen_frameCount = 0;
    recti_t m_captureScreen_region;
    std::function<void(Image&&)> m_captureScreen_callback = nullptr;

public:
    CommonRenderer() : m_builder(this) { }
    virtual GLFWwindow* Initialize(RendererOptions&& optionsIns) override;
    virtual void EnqueueCommands(const RenderBatch& batch) override;
    virtual void OnResize(int32_t width, int32_t height) override;
    virtual RenderBatchBuilder CreateCommandBuilder() override;
    virtual void Flush() override;
    virtual void RenderScope(std::function<void()>&& fn) override;
    virtual void CaptureScreen(uint32_t frameCount, std::function<void(Image&&)>&& fn) override;
    virtual void CaptureScreen(uint32_t frameCount, recti_t region, std::function<void(Image&&)>&& fn) override;
    virtual RenderStats GetStats() override;
    virtual RendererCapability GetCapabilities() const override;

    // Translation methods
    virtual StackGuard Transform(const m4_t& transform, bool multiply) override;
    virtual StackGuard TranslateTransfrom(float x, float y) override;
    virtual StackGuard RotateTransform(radians_t angle) override;
    virtual const m4_t& GetCurrentTransform() const override;

    [[nodiscard]] virtual StackGuard Clip(rectui_t region) override;

    // Drawing methods
    virtual void DrawBezierCubic(v2_t p1, v2_t p2, v2_t p3, v2_t p4, float width, xpf::Color color, LineOptions lineOptions, uint32_t detail) override;
    virtual void DrawBezierCubic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, const PolyLineVertex& v4, LineOptions lineOptions, uint32_t detail) override;
    virtual void DrawBezierQuadratic(v2_t p1, v2_t p2, v2_t p3, float width, xpf::Color color, LineOptions lineOptions, uint32_t detail) override;
    virtual void DrawBezierQuadratic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, LineOptions lineOptions, uint32_t detail) override;
    virtual void DrawCircle(float x, float y, float radius, xpf::Color color) override;
    virtual void DrawCircle(float x, float y, const CircleDescription& description) override;
    virtual void DrawImage(float x, float y, float w, float h, const std::shared_ptr<ITexture>& spTexture, const rectf_t& coords, xpf::Color color) override;
    virtual void DrawLine(float x0, float y0, float x1, float y1, float width, xpf::Color color, LineOptions lineOptions = LineOptions::Default) override;
    virtual void DrawLine(const std::vector<PolyLineVertex>& points, LineOptions lineOptions) override;
    virtual void DrawRectangle(float x, float y, const RectangleDescription& description) override;
    virtual void DrawRectangle(float x, float y, float w, float h, xpf::Color color) override;
    virtual void DrawRectangle(rectf_t rect, xpf::Color color) override;
    virtual rectf_t DrawText(std::string_view text, float x, float y, const TextDescription& description) override;
    virtual void DrawText(std::string_view text, float x, float y, std::string_view fontName, uint16_t fontSize, xpf::Color color) override;
    virtual void DrawText(float x, float y, FormattedText& ft, xpf::Color color) override;
};

} // xpf