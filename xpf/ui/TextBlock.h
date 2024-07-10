#pragma once
#include <xpf/renderer/common/FormattedText.h>
#include <xpf/ui/UIElement.h>

namespace xpf {

enum class TextVerticalAlignment {
    Top,
    Center,
    Bottom,
    CenterBaseline,
};

class TextBlock : public UIElement
{
protected:
    DECLARE_PROPERTY(TextBlock, std::string, Text, std::string(), Invalidates::ParentLayout);
    DECLARE_PROPERTY(TextBlock, std::string, FontName, std::string(), Invalidates::ParentLayout);
    DECLARE_PROPERTY(TextBlock, int16_t, FontSize, int16_t(15), Invalidates::ParentLayout);
    DECLARE_PROPERTY(TextBlock, bool, FontSizeIsInPixels, false, Invalidates::ParentLayout);
    DECLARE_PROPERTY(TextBlock, TextTrimming, TextTrimming, TextTrimming::Ellipsis, Invalidates::SelfLayout);
    DECLARE_PROPERTY(TextBlock, TextAlignment, TextAlignment, TextAlignment::Left, Invalidates::SelfLayout);
    DECLARE_PROPERTY(TextBlock, TextVerticalAlignment, TextVerticalAlignment, TextVerticalAlignment::Top, Invalidates::SelfLayout);

protected:
    FormattedText m_formattedText;
    rectf_t m_textRect;
    RenderBatch m_renderCommands;

public:
    TextBlock(UIElementType type = UIElementType::TextBlock)
        : UIElement(type)
    {
        m_clippingEnabled = false;
        m_pixel_perfect = true;
    }

    v2_t GetBounds() const { return m_formattedText.GetBounds(); }

    virtual v2_t OnMeasure(v2_t constraint) override
    {
        Typeface typeface;
        typeface.name = m_FontName;
        typeface.size = m_FontSize;
        if (m_FontSizeIsInPixels)
            typeface.renderOptions = FontRenderOptions::SizeInPixels;

        m_formattedText.SetText(m_Text.Get());
        m_formattedText.SetFont(typeface);
        m_formattedText.SetTextTrimming(m_TextTrimming);
        m_formattedText.SetMaxWidth(m_Width.ValueOr(constraint.x));
        m_formattedText.SetMaxHeight(m_Height.ValueOr(constraint.y));
        m_formattedText.BuildGeometry();

        return m_formattedText.GetBounds().round_up();
    }

    virtual void OnArrange(v2_t insideSize) override
    {
        rectf_t rect = m_insideRect;

        m_formattedText.BuildGeometry();
        v2_t bounds = m_formattedText.GetBounds();

        switch (m_TextAlignment)
        {
            case TextAlignment::Left:
            case TextAlignment::Justify:
                break;
            case TextAlignment::Center:
                rect.x += (rect.w - bounds.x) * .5;
                rect.w = bounds.x;
                break;
            case TextAlignment::Right:
                rect.x += rect.w - bounds.x;
                rect.w = bounds.x;
                break;
        }

        switch (m_TextVerticalAlignment)
        {
            case TextVerticalAlignment::Top:
                break;
            case TextVerticalAlignment::Center:
                rect.y += (rect.h - bounds.y) * .5;
                break;
            case TextVerticalAlignment::Bottom:
                if (rect.h > bounds.y)
                    rect.y = (rect.bottom() - bounds.y);
                break;
            case TextVerticalAlignment::CenterBaseline:
                rect.y += (rect.h - m_formattedText.GetBaseline()) * .5;
                break;
        }

        m_textRect = rect;
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        auto r = renderer.CreateCommandBuilder();
        r.DrawText(m_textRect.x, m_textRect.y, m_formattedText, m_Foreground);
        m_renderCommands = r.Build();
    }

    virtual void OnDraw(IRenderer& renderer) override
    {
        renderer.EnqueueCommands(m_renderCommands);
    }
};

} // xpf