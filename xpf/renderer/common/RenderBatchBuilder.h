#pragma once
#include <core/Macros.h>
#include <core/Types.h>
#include <renderer/common/Font.h>
#include <renderer/IRenderer.h>
#include <renderer/RenderCommand.h>
#include <math/m4_t.h>
#include <math/v4_t.h>
#include <vector>

namespace xpf {

#pragma pack(push)
#pragma pack(1)
struct VertexPositionColor
{
    v2_t position;
    v4_t color;
};

struct VertexPositionColorTextureCoords
{
    v2_t position;
    v4_t color;
    v2_t textureCoords;
};

struct GlyphInstanceData
{
    v4_t color;
    v2_t textureCoords;
    uint32_t startOffset;
    uint32_t length;
    float scale;
};
#pragma pack(pop)

struct Quad;
struct Color;

enum class HorizontalOrientation
{
    Left,
    Center,
    Right,
};

enum class VerticalOrientation
{
    Top,
    Center,
    Bottom,
};

struct CircleDescription
{
    float radius;
    float borderThickness;
    HorizontalOrientation horizontalOrientation = HorizontalOrientation::Left;
    VerticalOrientation verticalOrientation = VerticalOrientation::Top;
    xpf::Color fillColor;
    xpf::Color borderColor;
    std::shared_ptr<ITexture> spTexture;
};

struct RectangleDescription
{
    enum BorderType
    {
        Solid,
        Dot
    };

    float width = 0;
    float height = 0;
    HorizontalOrientation horizontalOrientation = HorizontalOrientation::Left;
    VerticalOrientation verticalOrientation = VerticalOrientation::Top;
    xpf::Color fillColor;
    xpf::Color borderColor;
    xpf::Thickness borderThickness;
    xpf::CornerRadius cornerRadius;
    std::shared_ptr<ITexture> spTexture;
    BorderType borderType = BorderType::Solid;
};

enum class TextTrimming
{
    None,
    Ellipsis,
};

struct TextDescription
{
    HorizontalOrientation horizontalOrientation = HorizontalOrientation::Left;
    VerticalOrientation verticalOrientation = VerticalOrientation::Top;
    xpf::Color foreground;
    xpf::Color background = xpf::Colors::Transparent;
    std::string fontName;
    int16_t fontSize = 10; // negative values for vector rendered fonts
    uint16_t fontIndex = 0;
    TextTrimming trimming = TextTrimming::None;
    FontRenderOptions renderOptions = FontRenderOptions::Texture;
};

struct PolyLineVertex
{
    typedef v2_t PointType;
    typedef xpf::Color ColorType;

    v2_t position;
    xpf::Color color;
    float width = 1;
};

class IRenderer;

class RenderBatchBuilder
{
friend class PolylineBackInserter;
protected:
    IRenderer* m_pRenderer;
    std::vector<float> m_vertices;
    std::vector<GlyphInstanceData> m_glyphInstanceData;
    uint32_t m_vertex_count = 0;
    uint32_t m_vertex_size = 6;
    std::shared_ptr<ITexture> m_spTexture;
    uint32_t m_codepage_id = 0;

    std::vector<std::shared_ptr<RenderCommand>> m_commands;
    RenderCommandId m_commandId = RenderCommandId::position;

    std::vector<PolyLineVertex> m_polylineTempCache;

    m4_t m_currentTransform = m4_t::identity;
    rectui_t m_currentClipRegion = rectui_t{0,0,UINT32_MAX, UINT32_MAX};

public:
    RenderBatchBuilder(IRenderer* pRenderer) : m_pRenderer(pRenderer) { }
    void AppendCommand(const std::shared_ptr<RenderCommand>& spCmd);
    RenderBatch Build();

    void Flush();
    void RunAction(std::function<RenderBatch()>&& fn);
    [[nodiscard]] StackGuard Transform(const m4_t& transform, bool multiply = true);
    [[nodiscard]] StackGuard TranslateTransfrom(float x, float y);
    [[nodiscard]] StackGuard RotateTransform(radians_t angle);
    const m4_t& GetCurrentTransform() const;
    [[nodiscard]] StackGuard Clip(rectui_t region);

    void DrawTriangle(v2_t p0, v2_t p1, v2_t p2, xpf::Color color);
    bool DrawBezierQuadraticTriangle(v2_t p0, v2_t ctrl, v2_t p1, xpf::Color color);
    void DrawBezierCubicQuad(v2_t p0, v2_t ctrl0, v2_t ctrl1, v2_t p1, xpf::Color color);

    // Drawing methods
    void DrawBezierCubic(v2_t p1, v2_t p2, v2_t p3, v2_t p4, float width, xpf::Color color, LineOptions lineOptions = LineOptions::Default, uint32_t detail = 40);
    void DrawBezierCubic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, const PolyLineVertex& v4, LineOptions lineOptions = LineOptions::Default, uint32_t detail = 40);
    void DrawBezierQuadratic(v2_t p1, v2_t p2, v2_t p3, float width, xpf::Color color, LineOptions lineOptions = LineOptions::Default, uint32_t detail = 40);
    void DrawBezierQuadratic(const PolyLineVertex& v1, const PolyLineVertex& v2, const PolyLineVertex& v3, LineOptions lineOptions = LineOptions::Default, uint32_t detail = 40);
    void DrawCircle(float x, float y, float radius, xpf::Color color);
    void DrawCircle(float x, float y, const CircleDescription& description);
    void DrawImage(float x, float y, float w, float h, const std::shared_ptr<ITexture>& spTexture, const rectf_t& coords = {0,0,1,1}, xpf::Color color = xpf::Colors::White);
    void DrawLine(float x0, float y0, float x1, float y1, float width, xpf::Color color, LineOptions lineOptions = LineOptions::Default);
    void DrawLine(const std::vector<PolyLineVertex>& points, LineOptions lineOptions = LineOptions::Default);
    void DrawRectangle(float x, float y, const RectangleDescription& description);
    void DrawRectangle(float x, float y, float w, float h, xpf::Color color);
    void DrawRectangle(rectf_t rect, xpf::Color color);
    rectf_t DrawText(std::string_view text, float x, float y, const TextDescription& description);
    void DrawText(std::string_view text, float x, float y, std::string_view fontName = "", uint16_t fontSize = 10, xpf::Color color = xpf::Colors::Black);
    void DrawText(float x, float y, FormattedText& ft, xpf::Color color);

    void DrawLine(v2_t p0, v2_t p1, float width, xpf::Color color, LineOptions lineOptions);

protected:
    void Push(v2_t pos, xpf::Color color);
    void Push3(v2_t p1, v2_t p2, v2_t p3, xpf::Color color);
    void PushQuad(v2_t p1, v2_t p2, v2_t p3, v2_t p4, xpf::Color color);
    void PushQuad(const xpf::Quad& quad, xpf::Color stroke);
    void Push(v2_t pos, v4_t color, v2_t textureCoords);
    void Push3(
        VertexPositionColorTextureCoords&& p0,
        VertexPositionColorTextureCoords&& p1,
        VertexPositionColorTextureCoords&& p2);
    void PushQuad(
        VertexPositionColorTextureCoords&& topLeft,
        VertexPositionColorTextureCoords&& topRight,
        VertexPositionColorTextureCoords&& bottomRight,
        VertexPositionColorTextureCoords&& bottomLeft);
};

} // xpf