#pragma once
#include <xpf/math/xpfmath.h>
#include <xpf/ui/UIElement.h>
#include <xpf/windows/InputService.h>
#include <xpf/ui/MouseClickTrackerMixIn.h>
#include <xpf/math/xpfmath.h>
#include <xpf/core/Tween.h>

namespace xpf {

struct ScrollbarAnimationData
{
    xpf::Color background;
    xpf::Color foreground;
    rectf_t splitterRect;
};

template<> struct math_trait<xpf::ScrollbarAnimationData>
{
    static xpf::ScrollbarAnimationData lerp(
        const xpf::ScrollbarAnimationData& a,
        const xpf::ScrollbarAnimationData& b,
        float t)
    {
        return ScrollbarAnimationData {
            xpf::math_trait<Color>::lerp(a.background, b.background, t),
            xpf::math_trait<Color>::lerp(a.foreground, b.foreground, t),
            xpf::math_trait<rectf_t>::lerp(a.splitterRect, b.splitterRect, t)
        };
    }
};

class Scrollbar : public MouseClickTrackerMixIn<UIElement>
{
#pragma region visuals
protected:
    rectf_t m_rect;
    rectf_t m_rect_hover;
    Tween<ScrollbarAnimationData> m_animator;
#pragma endregion

#pragma region properties
    DECLARE_PROPERTY(Scrollbar, Orientation, Orientation, Orientation::Vertical, Invalidates::ParentLayout);
    DECLARE_PROPERTY(Scrollbar, float, ViewportSize, 25, Invalidates::SelfLayout);
    DECLARE_PROPERTY_GETONLY(Scrollbar, float, Value, 0.0f, Invalidates::Visuals);
    DECLARE_PROPERTY(Scrollbar, float, MinValue, 0.0f, Invalidates::SelfLayout);
    DECLARE_PROPERTY(Scrollbar, float, MaxValue, 100.0f, Invalidates::SelfLayout);
    DECLARE_PROPERTY(Scrollbar, float, ScrollbarWidth, 16.0f, Invalidates::ParentLayout);
    DECLARE_PROPERTY(Scrollbar, float, SliderWidth, 2.0f, Invalidates::SelfLayout);
    DECLARE_PROPERTY(Scrollbar, float, SliderHoverWidth, 8.0f, Invalidates::SelfLayout);
    DECLARE_THEMED_PROPERTY(Scrollbar, xpf::Color, Background_Default, xpf::Colors::Black, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Scrollbar, xpf::Color, Background_Hover, xpf::Colors::Maroon, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Scrollbar, xpf::Color, Foreground_Default, xpf::Colors::Gray, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(Scrollbar, xpf::Color, Foreground_Hover, xpf::Colors::Red, Invalidates::Visuals);
#pragma endregion

public:
    Scrollbar()
        : MouseClickTrackerMixIn(UIElementType::Scrollbar)
    {
        m_Margin.SetIsReadOnly(true);
        m_Padding.SetIsReadOnly(true);
        m_CornerRadius.Set(m_ScrollbarWidth * .5);
    }

    Scrollbar& SetValue(float value)
    {
        float newval = xpf::math::clamp<float>(value, m_MinValue, m_MaxValue - m_ViewportSize);
        m_Value = newval;

        if (m_pParent != nullptr)
        {
            if (m_Orientation == Orientation::Vertical)
                m_pParent->SetVerticalScrollPosition(newval);
            else
                m_pParent->SetHorizontalScrollPosition(newval);
        }

        return *this;
    }

#pragma region layout
public:
    virtual v2_t OnMeasure(v2_t insideSize) override
    {
        if (m_Orientation == Orientation::Horizontal)
            return v2_t(m_Width.ValueOr(insideSize.x), m_ScrollbarWidth);

        return v2_t(m_ScrollbarWidth, m_Height.ValueOr(insideSize.y));
    }

    virtual void OnArrange(v2_t insideSize) override
    {
        m_rect = CalculateSliderRect(m_insideRect, m_SliderWidth);
        m_rect_hover = CalculateSliderRect(m_insideRect, m_SliderHoverWidth);
        m_Value = xpf::math::clamp<float>(m_Value, m_MinValue, m_MaxValue);

        return UIElement::OnArrange(insideSize);
    }

    rectf_t CalculateSliderRect(rectf_t rect, float sliderWidth) const
    {
        if (m_Orientation == Orientation::Vertical)
        {
            rect.x = rect.x + (rect.w - sliderWidth) * .5;
            rect.w = sliderWidth;
        }
        else
        {
            rect.y = rect.y + (rect.h - sliderWidth) * .5;
            rect.h = sliderWidth;
        }

        return rect;
    }
#pragma endregion

#pragma region drawing
protected:
    virtual void OnDraw(IRenderer& renderer) override
    {
        StateManager(renderer);

        rectf_t rect;
        if (m_animator.IsPlaying())
        {
            const ScrollbarAnimationData& data = m_animator.Read();
            m_Foreground.Set(data.foreground);
            m_Background.Set(data.background);
            rect = data.splitterRect;
        }
        else if (IsMouseInside() || m_tracking || m_focusGained)
        {
            rect = m_rect_hover;
        }
        else
        {
            rect = m_rect;
        }

        float x, y;

        RectangleDescription desc;
        desc.fillColor = m_Foreground;

        float halfScrollbarWidth = m_ScrollbarWidth * .5f;
        if (m_Orientation == Orientation::Vertical)
        {
            y = xpf::math::map_clamped<float>(m_Value, m_MinValue, m_MaxValue, rect.top() + halfScrollbarWidth, rect.bottom() - halfScrollbarWidth);
            float length = xpf::math::map_clamped<float>(m_ViewportSize, m_MinValue, m_MaxValue, 0, rect.height - m_ScrollbarWidth);
            if (length < rect.width)
                length = rect.width*2;
            x = rect.left();
            desc.width = rect.width, desc.height = length;
            desc.cornerRadius = rect.width * .5;
        }
        else
        {
            x = xpf::math::map_clamped<float>(m_Value, m_MinValue, m_MaxValue, rect.left() + halfScrollbarWidth, rect.right() - halfScrollbarWidth);
            float length = xpf::math::map_clamped<float>(m_ViewportSize, m_MinValue, m_MaxValue, 0, rect.width - m_ScrollbarWidth);
            if (length < rect.height)
                length = rect.height*2;
            y = rect.top();
            desc.width = length, desc.height = rect.height;
            desc.cornerRadius = rect.height * .5;
        }

        renderer.DrawRectangle(x, y, desc);
    }
#pragma endregion

protected:
    bool m_tracking = false;
    virtual void TrackingStarted() override { m_tracking = true; }
    virtual void TrackingStopped() override { m_tracking = false; }

    bool m_playingFocusGain = false;
    bool m_focusGained = false;
    bool m_playingFocusLoss = false;
    bool m_focusLost = true;

    void StateManager(IRenderer& renderer)
    {
        if (!GetIsHitTestVisible())
        {
            SetBackground(m_Background_Default);
            return;
        }

        v2_t pos = InputService::GetMousePosition();
        auto topleft = renderer.GetCurrentTransform().get_position();
        rectf_t rect = m_borderRect.move(topleft.x, topleft.y);
        bool isMouseInside = rect.is_inside(pos);
        if (!TrackMouse(isMouseInside))
            return;

        if (m_tracking)
        {
            OnLeftMouseButtonDown(pos, topleft);
            InvalidateLayout();
            isMouseInside = true;
        }

        if (isMouseInside)
        {
            if (m_playingFocusLoss)
                m_animator.Stop();

            m_playingFocusLoss = false;
            m_focusLost = false;

            if (m_animator.IsPlaying())
            {
                // noop
            }
            else if (m_focusGained)
            {
                m_Background.Set(m_Background_Hover);
                m_Foreground.Set(m_Foreground_Hover);
            }
            else if (!m_playingFocusGain)
            {
                m_animator.Stop();
                m_playingFocusGain = true;
                m_focusGained = false;
                m_animator
                    .Clear()
                    .From({m_Background_Default, m_Foreground_Default, m_rect})
                    .To({m_Background_Hover, m_Foreground_Hover, m_rect_hover}, .2f)
                    .Play()
                    .OnComplete([this](bool)
                    {
                        m_playingFocusGain = false;
                        m_focusGained = true;
                    });
            }
        }
        else
        {
            if (m_playingFocusGain)
                m_animator.Stop();

            m_playingFocusGain = false;
            m_focusGained = false;

            if (m_animator.IsPlaying())
            {
                // noop
            }
            else if (m_focusLost)
            {
                m_Background.Set(m_Background_Default);
                m_Foreground.Set(m_Foreground_Default);
            }
            else if (!m_playingFocusLoss)
            {
                m_animator.Stop();
                m_playingFocusLoss = true;
                m_focusLost = false;
                m_animator
                    .Clear()
                    .From({m_Background_Hover, m_Foreground_Hover, m_rect_hover})
                    .To({m_Background_Default, m_Foreground_Default, m_rect}, .5f)
                    .Play()
                    .OnComplete([this](bool)
                    {
                        m_playingFocusLoss = false;
                        m_focusLost = true;
                    });
            }
        }
    }

    void OnLeftMouseButtonDown(v2_t pos, v3_t topleft)
    {
        float newmidPoint;
        if (m_Orientation == Orientation::Vertical)
        {
            float screenpos = pos.y - topleft.y;
            newmidPoint = xpf::math::map_clamped<float>(screenpos, m_rect.top(), m_rect.bottom(), m_MinValue, m_MaxValue);
        }
        else
        {
            float screenpos = pos.x - topleft.x;
            newmidPoint = xpf::math::map_clamped<float>(screenpos, m_rect.left(), m_rect.right(), m_MinValue, m_MaxValue);
        }

        SetValue(newmidPoint - m_ViewportSize * .5f);
    }
};

} // xpf