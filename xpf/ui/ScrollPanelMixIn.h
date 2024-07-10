#pragma once
#include <xpf/ui/Scrollbar.h>
#include <xpf/core/Rectangle.h>

namespace xpf {

enum class ScrollOptions
{
    NoScroll,
    FittedScroll,
    ScrollOvershoot,
};

template<typename tbase>
class ScrollPanelMixIn : public tbase
{
protected:
    DECLARE_PROPERTY(ScrollPanelMixIn, ScrollOptions, ScrollOptions, ScrollOptions::NoScroll, Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, bool, VerticalScrollbarEnabled, true, Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, bool, HorizontalScrollbarEnabled, true, Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, bool, ShowVerticalScrollbar, false, Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, bool, ShowHorizontalScrollbar, false, Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, float, ScrollbarWidth, 16.0f, Invalidates::ParentLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, float, SliderWidth, 2.0f, Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, float, SliderHoverWidth, 8.0f, Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, v2_t, ContentSize, v2_t(), Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, v2_t, ViewPortSize, v2_t(), Invalidates::SelfLayout);
    DECLARE_PROPERTY(ScrollPanelMixIn, bool, OverlayScrollbarsOverContent, true, Invalidates::Visuals);

    std::unique_ptr<Scrollbar> m_spVertical;
    std::unique_ptr<Scrollbar> m_spHorizontal;

public:
    ScrollPanelMixIn(UIElementType type) : tbase(type) { }

    void HorizontalScrollTo(float val) { tbase::m_HorizontalScrollPosition.Set(val); }
    void VerticalScrollTo(float val) { tbase::m_VerticalScrollPosition.Set(val); }

protected:
    void ArrangeScrollbars(const rectf_t& rect)
    {
        if (m_ScrollOptions == ScrollOptions::NoScroll)
            return;

        v2_t contentSize = m_ContentSize;
        v2_t viewPortSize = m_ViewPortSize;
        const float scrollbarWidth = m_ScrollbarWidth.Get();

        float scrollbar_corner_width = 0;
        m_ShowHorizontalScrollbar = m_HorizontalScrollbarEnabled && (contentSize.x > viewPortSize.x);
        m_ShowVerticalScrollbar = m_VerticalScrollbarEnabled && (contentSize.y > viewPortSize.y);

        if (m_ShowHorizontalScrollbar)
        {
            if (m_spHorizontal == nullptr)
            {
                m_spHorizontal = std::make_unique<Scrollbar>();
                m_spHorizontal->SetOrientation(Orientation::Horizontal);
                this->AddVisual(m_spHorizontal.get());
            }

            scrollbar_corner_width = m_ShowVerticalScrollbar.Get() ? scrollbarWidth : 0;
            m_spHorizontal->SetMaxValue(
                contentSize.x + (m_ScrollOptions == ScrollOptions::FittedScroll ? 0 : rect.w * .5))
                .SetValue(tbase::GetHorizontalScrollPosition())
                .SetViewportSize(viewPortSize.x)
                .SetX(rect.left())
                .SetY(rect.bottom() - scrollbarWidth)
                .SetWidth(rect.w - scrollbar_corner_width)
                .SetHeight(scrollbarWidth);
        }
        else
        {
            tbase::SetHorizontalScrollPosition(0);
            m_spHorizontal = nullptr;
        }

        float scrollbar_corner_height = 0;
        if (m_ShowVerticalScrollbar)
        {
            if (m_spVertical == nullptr)
            {
                m_spVertical = std::make_unique<Scrollbar>();
                m_spVertical->SetOrientation(Orientation::Vertical);
                this->AddVisual(m_spVertical.get());
            }

            scrollbar_corner_height = (m_ShowHorizontalScrollbar.Get() ? scrollbarWidth : 0);
            m_spVertical->SetMaxValue(
                contentSize.y + (m_ScrollOptions == ScrollOptions::FittedScroll ? 0 : rect.h * .5))
                .SetValue(tbase::GetVerticalScrollPosition())
                .SetViewportSize(viewPortSize.y)
                .SetX(rect.right() - scrollbarWidth - 1)
                .SetY(rect.top())
                .SetHeight(rect.h - scrollbar_corner_height)
                .SetWidth(scrollbarWidth);
        }
        else
        {
            tbase::SetVerticalScrollPosition(0);
            m_spVertical = nullptr;
        }
    }

    virtual void OnMouse(bool is_inside) override
    {
        if (is_inside && m_ScrollOptions != ScrollOptions::NoScroll)
        {
            HorizontalScrollbarMouseWheelHandler();
            VerticalScrollbarMouseWheelHandler();
        }

        tbase::OnMouse(is_inside);
    }

    void VerticalScrollbarMouseWheelHandler()
    {
        if (m_spVertical == nullptr)
            return;

        float delta = -InputService::GetMouseWheelMove();
        if (math::is_negligible(delta)) return;

        m_spVertical->SetValue(m_spVertical->GetValue() + delta * 10);
    }

    void HorizontalScrollbarMouseWheelHandler()
    {
        if (m_spHorizontal == nullptr)
            return;

        float delta = -InputService::GetMouseWheelMove();
        if (math::is_negligible(delta)) return;

        m_spHorizontal->SetValue(m_spHorizontal->GetValue() + delta * 10);
    }

    void DrawScrollbars(IRenderer& renderer)
    {
        v2_t contentSize = m_ContentSize;
        v2_t viewPortSize = m_ViewPortSize;

        if(m_ShowVerticalScrollbar && m_spVertical != nullptr && contentSize.y > viewPortSize.y)
            m_spVertical->Draw(renderer);

        if(m_ShowHorizontalScrollbar && m_spHorizontal != nullptr && contentSize.x > viewPortSize.x)
            m_spHorizontal->Draw(renderer);
    }
};

} // xpf