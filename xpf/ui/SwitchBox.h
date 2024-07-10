#pragma once
#include <xpf/ui/UIElement.h>

namespace xpf {

enum class SwitchBoxState : uint8_t
{
    Default  = 0x0,
    Hover    = 0x1,
    Focused  = 0x2,
};

ENUM_CLASS_FLAG_OPERATORS(SwitchBoxState);

class SwitchBox : public UIElement
{
protected:
    DECLARE_PROPERTY(SwitchBox, std::string, FontName, std::string(), Invalidates::ParentLayout);
    DECLARE_PROPERTY(SwitchBox, int16_t, FontSize, int16_t(15), Invalidates::ParentLayout);
    DECLARE_PROPERTY(SwitchBox, bool, FontSizeIsInPixels, false, Invalidates::ParentLayout);
    DECLARE_PROPERTY(SwitchBox, Thickness, ItemMargin, 0, Invalidates::ParentLayout);
    DECLARE_PROPERTY(SwitchBox, Orientation, Orientation, Orientation::Horizontal, Invalidates::ParentLayout);
    DECLARE_PROPERTY(SwitchBox, float, SwitchDuration, .25f, Invalidates::None);
    DECLARE_PROPERTY(SwitchBox, uint32_t, SelectedItem, 0, Invalidates::SelfLayout);

    DECLARE_THEMED_PROPERTY(SwitchBox, xpf::Color, Foreground_Default, xpf::Colors::XpfWhite, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(SwitchBox, xpf::Color, Foreground_Focused, xpf::Colors::White, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(SwitchBox, xpf::Color, Foreground_Hover, xpf::Colors::White, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(SwitchBox, xpf::Color, Foreground_Selected, xpf::Colors::WhiteSmoke, Invalidates::Visuals);

    DECLARE_THEMED_PROPERTY(SwitchBox, xpf::Color, Background_Default, xpf::Colors::DodgerBlue.with_alpha(80), Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(SwitchBox, xpf::Color, Background_Focused, xpf::Colors::DodgerBlue.with_alpha(80), Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(SwitchBox, xpf::Color, Background_Hover, xpf::Colors::DodgerBlue.with_alpha(80), Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(SwitchBox, xpf::Color, Background_Selected, xpf::Colors::DodgerBlue, Invalidates::Visuals);

    struct Option
    {
        std::shared_ptr<ITexture> spIcon;
        FormattedText formattedText;
        xpf::Color foreground;
        xpf::Color background;
        bool iconOnly = false;
        rectf_t rect;
    };

    std::vector<Option> m_options;
    uint32_t m_selected = 0;
    CornerRadius m_corner;
    CornerRadius m_selector_corner;
    float m_height = 0;
    float m_width = 0;
    SwitchBoxState m_state = SwitchBoxState::Default;

public:
    SwitchBox() : UIElement(UIElementType::SwitchBox)
    {
        m_acceptsFocus = true;
        m_SelectedItem = 0;

        m_HorizontalAlignment.Set(HorizontalAlignment::Left);
        m_VerticalAlignment.Set(VerticalAlignment::Top);
        m_CornerRadius.Set(CornerRadius::AutoRadius());
    }

    uint32_t AddOption(const std::shared_ptr<ITexture>& spIcon, const std::string& text, xpf::Color foreground = xpf::Colors::Transparent, xpf::Color background = xpf::Colors::Transparent, bool isIconOnly = false)
    {
        FormattedText ft;
        ft.SetText(text);
        m_options.push_back(Option{spIcon, std::move(ft), foreground, background, isIconOnly});
        return m_options.size();
    }

    virtual v2_t OnMeasure(v2_t insideSize) override
    {
        if (!m_options.empty())
        {
            if (!m_SelectedItem.IsSet())
                m_SelectedItem = 0;
            else if (m_SelectedItem >= m_options.size())
                m_SelectedItem = m_options.size() - 1;
        }

        Typeface typeface;
        typeface.name = m_FontName;
        typeface.size = m_FontSize;
        if (m_FontSizeIsInPixels)
            typeface.renderOptions = FontRenderOptions::SizeInPixels;

        float totalHeight = 0;
        float totalWidth = 0;
        float x = 0;
        float y = 0;

        const Thickness& margin = m_ItemMargin.Get();
        if (m_Orientation == Orientation::Horizontal)
        {
            for (auto& option : m_options)
            {
                option.formattedText.SetFont(typeface);
                option.formattedText.BuildGeometry();
                option.rect.x = x;
                option.rect.w = option.formattedText.GetBounds().x + margin.left_right();
                option.rect.h = option.formattedText.GetBounds().y + margin.top_bottom();
                x += option.rect.w;

                totalHeight = std::max(totalHeight, option.rect.h);
            }
            totalWidth = x;

            for (auto& option : m_options)
            {
                option.rect.y = y + (totalHeight - option.rect.h) * .5;
            }
        }
        else
        {
            for (auto& option : m_options)
            {
                option.formattedText.SetFont(typeface);
                option.formattedText.BuildGeometry();
                option.rect.y = y;
                option.rect.w = option.formattedText.GetBounds().x + margin.left_right();
                option.rect.h = option.formattedText.GetBounds().y + margin.top_bottom();
                y += option.rect.h;

                totalWidth = std::max(totalWidth, option.rect.w);
            }
            totalHeight = y;

            for (auto& option : m_options)
            {
                option.rect.x = x + (totalWidth - option.rect.w) * .5;
            }
        }

        m_corner = (m_CornerRadius != CornerRadius::AutoRadius())
            ? m_CornerRadius
            : m_Orientation == Orientation::Horizontal || m_options.empty()
                ? CornerRadius::AutoRadius()
                : (m_options[0].rect.height == m_options[0].rect.width ? CornerRadius::AutoRadius() : (m_options[0].rect.height * .5));

        m_selector_corner = (m_CornerRadius != CornerRadius::AutoRadius())
            ? m_CornerRadius
            : m_Orientation == Orientation::Horizontal || m_options.empty()
                ? CornerRadius::AutoRadius()
                : (m_options[0].rect.height == m_options[0].rect.width ? CornerRadius::AutoRadius() : (m_options[0].rect.height * .5));

        return {totalWidth, totalHeight};
    }

    virtual void OnArrange(v2_t insideSize) override
    {
        for (auto& option : m_options)
        {
            option.rect.x += m_insideRect.x;
            option.rect.y += m_insideRect.y;
        }
    }

    virtual RenderBatch BuildBackground(IRenderer& renderer) override
    {
        xpf::Color backgroundColor = m_Background_Default.IsSet() ? m_Background_Default.Get() : m_Background.Get();
        if (m_state & SwitchBoxState::Hover)
            backgroundColor = backgroundColor.is_transparent() ? m_Background_Hover : backgroundColor.mix(m_Background_Hover, .5);

        if (m_state & SwitchBoxState::Focused)
            backgroundColor = backgroundColor.is_transparent() ? m_Background_Focused : backgroundColor.mix(m_Background_Focused, .5);

        auto r = renderer.CreateCommandBuilder();
        r.DrawRectangle(
            m_borderRect.x,
            m_borderRect.y, {
            m_borderRect.width,
            m_borderRect.height,
            HorizontalOrientation::Left,
            VerticalOrientation::Top,
            backgroundColor,
            m_Border,
            m_BorderThickness,
            m_corner});

        return r.Build();
    }

    Tween<float> m_selectionAnimation;

    virtual void DrawFocus(IRenderer& renderer) override
    {
        if (IsInFocus() && !m_options.empty())
        {
            rectangle rect = m_paddingRect.shrink(1);
            renderer.DrawRectangle(
                rect.x,
                rect.y, {
                rect.width,
                rect.height,
                HorizontalOrientation::Left,
                VerticalOrientation::Top,
                xpf::Colors::Transparent,
                m_focusBorder.Get(),
                1.0,
                m_corner,
                nullptr,
                RectangleDescription::Dot
                });
        }
    }

    virtual void OnDraw(IRenderer& renderer) override
    {
        StateManager(renderer);

        if (m_selected != m_SelectedItem && !m_selectionAnimation.IsPlaying())
        {
            m_selectionAnimation
                .Clear().From(0).To(1.0, m_SwitchDuration.Get(), xpf::math::cubic_ease_inOut)
                .OnComplete([&](bool){ m_selected = m_SelectedItem; })
                .Play();
        }

        float t = m_selectionAnimation.Read();
        const auto& prev = m_options[m_selected];
        const auto& sele = m_options[m_SelectedItem];

        xpf::Color prevground = prev.background.is_transparent() ? m_Background_Selected.Get() : prev.background;
        xpf::Color background = sele.background.is_transparent() ? m_Background_Selected.Get() : sele.background;

        RectangleDescription desc;
        desc.cornerRadius = m_selector_corner;
        desc.fillColor = xpf::math::lerp(prevground, background, t);
        desc.width = xpf::math::lerp<float>(prev.rect.width, sele.rect.width, t);
        desc.height = xpf::math::lerp<float>(prev.rect.height, sele.rect.height, t);
        renderer.DrawRectangle(
            xpf::math::lerp<float>(prev.rect.x, sele.rect.x, t),
            xpf::math::lerp<float>(prev.rect.y, sele.rect.y, t),
            desc);

        const Thickness& margin = m_ItemMargin.Get();
        uint32_t i = 0;
        v2_t mousePos = GetMousePosition(renderer);
        for (auto& option : m_options)
        {
            bool hovering = false;
            if (option.rect.is_inside(mousePos))
            {
                RectangleDescription desc;
                desc.cornerRadius = m_selector_corner;
                desc.fillColor = m_Background_Hover;
                desc.width = option.rect.width;
                desc.height = option.rect.height;
                renderer.DrawRectangle(option.rect.x, option.rect.y, desc);

                hovering = true;
            }

            xpf::Color foreground = option.foreground.is_transparent()
                ? (i == m_SelectedItem ? m_Foreground_Selected : m_Foreground_Default)
                : option.foreground;

            renderer.DrawText(
                margin.l + option.rect.x,
                margin.t + option.rect.y,
                option.formattedText, foreground);

            i++;
        }
    }

protected:
    void StateManager(IRenderer& renderer)
    {
        SwitchBoxState newState = SwitchBoxState::Default;
        if (IsInFocus())
        {
            newState |= SwitchBoxState::Focused;
            if (s_right.IsKeyPressed() && m_selected < m_options.size() - 1)
                m_SelectedItem = m_selected + 1;
            else if (s_left.IsKeyPressed() && m_selected > 0)
                m_SelectedItem = m_selected - 1;
        }

        if (IsMouseInside())
        {
            newState |= SwitchBoxState::Hover;
            if (InputService::IsMouseButtonDown(MouseButton::Left))
            {
                v2_t mousePos = GetMousePosition(renderer);
                for (uint32_t i = 0; i < m_options.size(); i++)
                {
                    if (m_options[i].rect.is_inside(mousePos))
                    {
                        m_SelectedItem = i;
                        return;
                    }
                }
            }
        }
    }
};

} // xpf