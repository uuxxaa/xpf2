#pragma once
#include <functional>
#include <memory>
#include <string>
#include <stdint.h>
#include <core/Color.h>
#include <core/CornerRadius.h>
#include <core/Rectangle.h>
#include <core/StackGuard.h>
#include <core/Thickness.h>
#include <core/Macros.h>
#include <math/m4_t.h>
#include <renderer/RenderCommand.h>

typedef uint8_t byte_t;
struct GLFWwindow;

namespace xpf {

class Image;
class IBuffer;
class ITexture;
class Font;
class FormattedText;

enum class RendererType
{
    Null,
    DirectX,
    OpenGL,
    Metal,
};

enum class RendererCapability : uint64_t
{
    Default = 0x0,
    ShaderRenderedText = 0x1,
    ShaderInstancedRenderedText = 0x2,
};

ENUM_CLASS_FLAG_OPERATORS(RendererCapability);

struct RendererOptions
{
    RendererType renderer;
    std::string title;
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t api_version_major = 0;
    uint8_t api_version_minor = 0;
    uint8_t sample_count = 0;
    bool is_projection_ortho2d = false;
    bool enable_vsync = false;
    bool show_stats = false;
    bool metal_transactional_rendering = true;
    xpf::Color foreground_color = xpf::Colors::XpfBlack;
    xpf::Color background_color = xpf::Colors::XpfWhite;
    RendererCapability capabilities = RendererCapability::Default;
};

enum class LineOptions : uint16_t
{
    Default,
    CapButt    = 0x0001,
    CapRound   = 0x0002,
    CapSquare  = 0x0004,
    // gap
    JointMiter = 0x0010,
    JointBevel = 0x0020,
    JointRound = 0x0040,
    // gap
    Closed     = 0x100,
};
ENUM_CLASS_FLAG_OPERATORS(LineOptions);

class RenderBatchBuilder;
struct CircleDescription;
struct RectangleDescription;
struct TextDescription;
struct PolyLineVertex;

struct RenderStats
{
    uint32_t drawCount = 0;
    uint32_t vertexCount = 0;
    uint32_t textureSwitches = 0;
    uint32_t bufferSwitches = 0;
};

class IRenderer
{
public:
    virtual ~IRenderer() {}
    virtual GLFWwindow* Initialize(RendererOptions&& options) = 0;
    virtual void Shutdown() = 0;
    virtual void OnResize(int32_t width, int32_t height) = 0;
    virtual void Render() = 0;
    virtual RenderStats GetStats() = 0;
    virtual RendererCapability GetCapabilities() const = 0;
    virtual void EnqueueCommands(const RenderBatch& batch) = 0;
    virtual void Flush() = 0;
    virtual RenderBatchBuilder CreateCommandBuilder() = 0;
    virtual void RenderScope(std::function<void()>&& fn) = 0;
    virtual void CaptureScreen(uint32_t frameCount, std::function<void(Image&&)>&& fn) = 0;
    virtual void CaptureScreen(uint32_t frameCount, recti_t rect, std::function<void(Image&&)>&& fn) = 0;

    // Translation methods
    [[nodiscard]] virtual StackGuard Transform(const m4_t& transform, bool multiply = true) = 0;
    [[nodiscard]] virtual StackGuard TranslateTransfrom(float x, float y) = 0;
    [[nodiscard]] virtual StackGuard RotateTransform(radians_t radians_t) = 0;
    virtual const m4_t& GetCurrentTransform() const = 0;

    [[nodiscard]] virtual StackGuard Clip(rectui_t region) = 0;

    // Drawing methods
    virtual void DrawBezierCubic(v2_t p1, v2_t p2, v2_t p3, v2_t p4, float width, xpf::Color color, LineOptions lineOptions = LineOptions::Default, uint32_t detail = 40) = 0;
    virtual void DrawBezierCubic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, const PolyLineVertex& v4, LineOptions lineOptions = LineOptions::Default, uint32_t detail = 40) = 0;
    virtual void DrawBezierQuadratic(v2_t p1, v2_t p2, v2_t p3, float width, xpf::Color color, LineOptions lineOptions = LineOptions::Default, uint32_t detail = 40) = 0;
    virtual void DrawBezierQuadratic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, LineOptions lineOptions = LineOptions::Default, uint32_t detail = 40) = 0;
    virtual void DrawCircle(float x, float y, float r, xpf::Color color) = 0;
    virtual void DrawCircle(float x, float y, const CircleDescription& description) = 0;
    virtual void DrawImage(float x, float y, float w, float h, const std::shared_ptr<ITexture>& spTexture, const rectf_t& coords = {0,0,1,1}, xpf::Color color = xpf::Colors::White) = 0;
    virtual void DrawLine(float x0, float y0, float x1, float y1, float width, xpf::Color color, LineOptions lineOptions = LineOptions::Default) = 0;
    virtual void DrawLine(const std::vector<PolyLineVertex>& points, LineOptions lineOptions = LineOptions::Default) = 0;
    virtual void DrawRectangle(float x, float y, const RectangleDescription& description) = 0;
    virtual void DrawRectangle(float x, float y, float w, float h, xpf::Color color) = 0;
    virtual void DrawRectangle(rectf_t, xpf::Color color) = 0;
    virtual rectf_t DrawText(std::string_view text, float x, float y, const TextDescription& description) = 0;
    virtual void DrawText(std::string_view text, float x, float y, std::string_view fontName = "", uint16_t fontSize = 10, xpf::Color color = xpf::Colors::Black) = 0;
    virtual void DrawText(float x, float y, FormattedText& ft, xpf::Color color) = 0;

    virtual std::shared_ptr<ITexture> CreateTexture(std::string_view filename) = 0;
    virtual std::shared_ptr<ITexture> CreateTexture(const Image& img) = 0;
    virtual std::shared_ptr<IBuffer> CreateBuffer(const byte_t* pbyte, size_t size) = 0;
};

std::unique_ptr<IRenderer> create_directx_renderer();
std::unique_ptr<IRenderer> create_opengl_renderer();
std::unique_ptr<IRenderer> create_metal_renderer();
std::unique_ptr<IRenderer> create_null_renderer();

} // xpf
