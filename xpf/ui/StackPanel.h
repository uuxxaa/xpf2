#pragma once
#include <xpf/ui/Panel.h>
#include <xpf/ui/ScrollPanelMixIn.h>
#include <xpf/math/hash.h>

namespace xpf {

class StackPanel : public ScrollPanelMixIn<Panel>
{
protected:
    DECLARE_PROPERTY(StackPanel, xpf::Orientation, Orientation, Orientation::Vertical, Invalidates::ParentLayout);

public:
    StackPanel() : ScrollPanelMixIn(UIElementType::StackPanel)
    {
        m_clippingEnabled = true;
    }

#pragma region measure & arrange
public:
    virtual v2_t OnMeasure(v2_t insideSize) override
    {
        float x = 0;
        float y = 0;
        bool isVertical = (m_Orientation == Orientation::Vertical);

        if (isVertical)
        {
            float height_stars = 0;
            float height_pixels = 0;
            std::unordered_map<UIElement*, float> heights;

            for (const auto& sp : m_children)
            {
                const PanelLength* pheight = sp->Get<Panel_Length>();
                if (pheight == nullptr || pheight->type == PanelLength::Auto)
                {
                    float height = sp->Measure(insideSize).y;
                    heights[sp.get()] = height;
                    height_pixels += height;
                }
                else if (pheight->type == PanelLength::Star)
                {
                    height_stars += pheight->length;
                }
                else
                {
                    height_pixels += pheight->length;
                }
            }

            float height_pixels_removed = (insideSize.y - height_pixels);
            if (height_pixels_removed < 0)
                height_pixels_removed = insideSize.y;

            float height_per_star = height_stars > 0 ? height_pixels_removed / height_stars : 0;

            for (const auto& sp : m_children)
            {
                const PanelLength* pheight = sp->Get<Panel_Length>();

                v2_t suggested_size = insideSize;
                if (pheight == nullptr || pheight->type == PanelLength::Auto)
                {
                    suggested_size.y = heights[sp.get()];
                }
                else if (pheight->type == PanelLength::Star)
                {
                    suggested_size.y = pheight->length * height_per_star;
                }
                else
                {
                    suggested_size.y = pheight->length;
                }

                v2_t desiredSize = sp->Measure(suggested_size);
                switch (sp->GetHorizontalAlignment())
                {
                    case HorizontalAlignment::Stretch:
                        desiredSize.x = insideSize.w;
                        break;
                    default:
                        break;
                }
                x = std::max(x, desiredSize.x);
                y += desiredSize.y;
            }
        }
        else // isHorizontal
        {
            float width_stars = 0;
            float width_pixels = 0;
            std::unordered_map<UIElement*, float> widths;

            for (const auto& spChild : m_children)
            {
                const PanelLength* pwidth = spChild->Get<Panel_Length>();
                if (pwidth == nullptr || pwidth->type == PanelLength::Auto)
                {
                    float width = spChild->Measure(insideSize).x;
                    widths[spChild.get()] = width;
                    width_pixels += width;
                }
                else if (pwidth->type == PanelLength::Star)
                {
                    width_stars += pwidth->length;
                }
                else
                {
                    width_pixels += pwidth->length;
                }
            }

            float width_pixels_removed = (insideSize.x - width_pixels);
            if (width_pixels_removed < 0)
                width_pixels_removed = insideSize.x;

            float width_per_star = width_stars > 0 ? width_pixels_removed / width_stars : 0;

            for (const auto& spChild : m_children)
            {
                const PanelLength* pwidth = spChild->Get<Panel_Length>();

                v2_t suggested_size = insideSize;
                if (pwidth == nullptr || pwidth->type == PanelLength::Auto)
                {
                    suggested_size.x = widths[spChild.get()];
                }
                else if (pwidth->type == PanelLength::Star)
                {
                    suggested_size.x = pwidth->length * width_per_star;
                }
                else
                {
                    suggested_size.x = pwidth->length;
                }

                v2_t desiredSize = spChild->Measure(suggested_size);
                switch (spChild->GetVerticalAlignment())
                {
                    case VerticalAlignment::Stretch:
                        desiredSize.y = insideSize.h;
                        break;
                    default:
                        break;
                }
                x += desiredSize.x;
                y = std::max(y, desiredSize.y);
            }
        }

        m_ContentSize.Set(v2_t(x, y));
        return m_ContentSize;
    }

    virtual void OnArrange(v2_t insideSize) override
    {
        float x = m_insideRect.x;
        float y = m_insideRect.y;
        float insideWidth = insideSize.w;
        float insideHeight = insideSize.h;

        if (!m_OverlayScrollbarsOverContent)
        {
            if (m_ShowHorizontalScrollbar)
                insideHeight -= m_ScrollbarWidth;

            if (m_ShowVerticalScrollbar)
                insideWidth -= m_ScrollbarWidth;
        }
        
        bool isVertical = m_Orientation == Orientation::Vertical;
        for (const auto& sp : m_children)
        {
            float left = x, top = y;
            const v2_t childSize = sp->GetDesiredSize() + sp->GetMargin();
            float w = childSize.x;
            float h = childSize.y;

            if (isVertical)
            {
                switch (sp->GetHorizontalAlignment())
                {
                    case HorizontalAlignment::Left: break;
                    case HorizontalAlignment::Center: left += (insideWidth - w) * .5; break;
                    case HorizontalAlignment::Right: left += insideWidth - w; break;
                    case HorizontalAlignment::Stretch: w = insideWidth; break;
                    //case horizontal_alignment::stretch: w = rect.w; break;
                }
            }
            else
            {
                switch (sp->GetVerticalAlignment())
                {
                    case VerticalAlignment::Top: break;
                    case VerticalAlignment::Center: top += (insideHeight - h) * .5; break;
                    case VerticalAlignment::Bottom: top += insideHeight - h; break;
                    case VerticalAlignment::Stretch: h = insideHeight; break;
                }
            }

            sp->Arrange({left, top, w, h});

            if (isVertical)
                y += h;
            else
                x += w;
        }

        m_ViewPortSize.Set(m_insideRect.size());
        ArrangeScrollbars(m_insideRect);
    }
#pragma endregion

#pragma region drawing
    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        for (const auto& sp : m_children)
            sp->UpdateVisuals(renderer);

        ScrollPanelMixIn<Panel>::OnUpdateVisuals(renderer);
    }

    virtual void OnDraw(IRenderer& renderer) override
    {
        if (m_Orientation == Orientation::Vertical)
        {
            float y = 0;
            float viewport_top = std::floor(m_VerticalScrollPosition);
            float viewport_height = m_insideRect.height;
            float viewport_bottom = std::ceil(viewport_top + viewport_height);
            auto scope = renderer.TranslateTransfrom(-std::floor(m_HorizontalScrollPosition), -viewport_top);

            for (const auto& sp : m_children)
            {
                y += sp->GetActualRect().height;
                if (y < viewport_top)
                    continue;

                sp->Draw(renderer);

                if (y > viewport_bottom)
                    break;
            }
        }
        else
        {
            float x = 0;
            float viewport_left = std::floor(m_HorizontalScrollPosition);
            float viewport_width = m_insideRect.width;
            float viewport_right = viewport_left + viewport_width;
            auto scope = renderer.TranslateTransfrom(-viewport_left, -std::floor(m_VerticalScrollPosition));

            for (const auto& sp : m_children)
            {
                x += sp->GetActualRect().width;
                if (x < viewport_left)
                    continue;

                sp->Draw(renderer);

                if (x > viewport_right)
                    break;
            }
        }

        DrawScrollbars(renderer);
    }
#pragma endregion

protected:
    virtual void OnAllChildrenRemoved() override {
        VerticalScrollTo(0);
        HorizontalScrollTo(0);
    }

    virtual void OnMouse(bool is_inside) override
    {
        if (is_inside && m_ScrollOptions != ScrollOptions::NoScroll)
        {
            if (m_Orientation == Orientation::Vertical)
                VerticalScrollbarMouseWheelHandler();
            else
                HorizontalScrollbarMouseWheelHandler();
        }

        Panel::OnMouse(is_inside);
    }
};

} // xpf
