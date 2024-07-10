#pragma once
#include <memory>
#include <vector>
#include <stdint.h>
#include <core/CornerRadius.h>
#include <math/m4_t.h>
#include <renderer/IBuffer.h>
#include <renderer/ITexture.h>

namespace xpf {

enum class RenderCommandId
{
    position,
    position_color,
    position_color_texture,
    text,
    rounded_rectangle,
    rounded_rectangle_with_border,
    bezier_quadratic_triangle,
    bezier_quadratic_triangle_ccw,
    bezier_cubic_quad,
    bezier_cubic_quad_ccw,
    glyph,
    glyphs,
    rounded_rectangle_with_border_dots,
    // transform should be last
    transform,
    clip,
    callback,
};

struct RenderCommand
{
    RenderCommandId commandId;
    RenderCommand(RenderCommandId idIn) : commandId(idIn) {}
};

struct RenderDrawCommand : public RenderCommand
{
    std::vector<float> verts;
    uint32_t count = 0;
    std::shared_ptr<ITexture> spTexture;
    RenderDrawCommand(
        RenderCommandId idIn,
        std::vector<float>&& vertsIn,
        uint32_t countIn,
        const std::shared_ptr<ITexture>& spTextureIn = nullptr)
        : RenderCommand(idIn)
        , verts(std::move(vertsIn))
        , count(countIn)
        , spTexture(spTextureIn)
    {}
};

struct RenderBatch
{
    std::vector<std::shared_ptr<RenderCommand>> commands;
    void ForEachCommand(const std::function<void(const std::shared_ptr<RenderCommand>&)>& fn) const;

    bool IsEmpty() const { return commands.empty(); }
    void Clear() { return commands.clear(); }
};

struct RenderTransformCommand : public RenderCommand
{
    enum TransformType
    {
        MultiplyPush,
        Push,
        Pop,
    };

    m4_t transform;
    TransformType type;

    RenderTransformCommand(const m4_t& transformIn, TransformType typeIn)
        : RenderCommand(RenderCommandId::transform)
        , transform(transformIn)
        , type(typeIn)
    {}

    RenderTransformCommand(m4_t&& transformIn, TransformType typeIn)
        : RenderCommand(RenderCommandId::transform)
        , transform(std::move(transformIn))
        , type(typeIn)
    {}
};

struct RenderClipCommand : public RenderCommand
{
    enum Type
    {
        Push,
        Pop,
    };

    rectui_t clipRegion;
    Type type;

    RenderClipCommand(rectui_t clipRegionIn, Type typeIn)
        : RenderCommand(RenderCommandId::clip)
        , clipRegion(clipRegionIn)
        , type(typeIn)
    {}
};

struct RenderRoundedRectangleCommand : public RenderDrawCommand
{
    v2_t size;
    CornerRadius cornerRadius;
    Thickness borderThickness;
    v4_t borderColor;
    bool borderDot = false;

    RenderRoundedRectangleCommand(
        std::vector<float>&& vertsIn,
        uint32_t countIn,
        v2_t sizeIn,
        const CornerRadius& cornerRadiusIn,
        const Thickness& borderThicknessIn,
        v4_t borderColorIn,
        const std::shared_ptr<ITexture>& spTextureIn = nullptr,
        bool borderDotIn = false)
        : RenderDrawCommand(
            borderThicknessIn.is_zero_or_negative()
                ? RenderCommandId::rounded_rectangle
                : (borderDotIn 
                    ? RenderCommandId::rounded_rectangle_with_border_dots
                    : RenderCommandId::rounded_rectangle_with_border),
                std::move(vertsIn), countIn, spTextureIn)
        , size(sizeIn)
        , cornerRadius(cornerRadiusIn)
        , borderThickness(borderThicknessIn)
        , borderColor(borderColorIn)
        , borderDot(borderDotIn)
    {}
};

struct RenderGlyphCommand : public RenderDrawCommand
{
    std::shared_ptr<IBuffer> spBuffer;
    uint32_t glyphStartOffset;
    float scale;
    RenderGlyphCommand(
        std::vector<float>&& vertsIn,
        uint32_t countIn,
        const std::shared_ptr<IBuffer>& spBufferIn,
        uint32_t startOffsetIn,
        float scaleIn)
        : RenderDrawCommand(RenderCommandId::glyph, std::move(vertsIn), countIn, nullptr)
        , spBuffer(spBufferIn)
        , glyphStartOffset(startOffsetIn)
        , scale(scaleIn)
    {}
};

struct RenderGlyphsCommand : public RenderDrawCommand
{
#pragma pack(push)
#pragma pack(1)
    struct GlyphsInstanceData
    {
        v4_t color;
        float left, top;
        float right, bottom;
        int32_t bearingX;
        int32_t width;
        int32_t bearingY;
        int32_t height;
        uint32_t dataStart;
        uint32_t padding0 = 0, padding1 = 0, padding2 = 0;
    };
#pragma pack(pop)

    std::vector<GlyphsInstanceData> instanceData;
    std::shared_ptr<IBuffer> spBuffer;
    uint32_t glyphCount = 0;
    float scale;
    RenderGlyphsCommand(
        std::vector<float>&& vertsIn,
        std::vector<GlyphsInstanceData>&& instanceDataIn,
        const std::shared_ptr<IBuffer>& spBufferIn,
        uint32_t glyphCountIn,
        float scaleIn)
        : RenderDrawCommand(RenderCommandId::glyphs, std::move(vertsIn), 4, nullptr)
        , instanceData(std::move(instanceDataIn))
        , spBuffer(spBufferIn)
        , glyphCount(glyphCountIn)
        , scale(scaleIn)
    {}
};

struct RenderCallbackCommand : public RenderCommand
{
    std::function<RenderBatch()> fn;
    RenderCallbackCommand(std::function<RenderBatch()>&& fnIn)
    : RenderCommand(RenderCommandId::callback)
    , fn(fnIn)
    {}
};

inline void RenderBatch::ForEachCommand(const std::function<void(const std::shared_ptr<RenderCommand>&)>& fn) const {
    for (const auto& spCommand : commands) {
        if (spCommand->commandId == RenderCommandId::callback)
        {
            const RenderCallbackCommand& command = *static_cast<RenderCallbackCommand*>(spCommand.get());
            RenderBatch batch = command.fn();
            batch.ForEachCommand(fn);
        }
        else
        {
            fn(spCommand);
        }
    }
}

} // xpf
