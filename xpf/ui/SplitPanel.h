#pragma once
#include <xpf/ui/Panel.h>
#include <xpf/ui/StackPanel.h>
#include <xpf/ui/MouseClickTrackerMixIn.h>
#include <xpf/core/Tween.h>

namespace xpf {

class SplitPanel : public MouseClickTrackerMixIn<Panel>
{
protected:
    DECLARE_PROPERTY(SplitPanel, xpf::Orientation, Orientation, Orientation::Vertical, Invalidates::SelfLayout);
    DECLARE_PROPERTY(SplitPanel, float, SplitterWidth, 8, Invalidates::SelfLayout);
    DECLARE_PROPERTY(SplitPanel, xpf::Color, SplitterBackground, xpf::Colors::Transparent, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(SplitPanel, xpf::Color, Background_Default, xpf::Colors::Transparent, Invalidates::Visuals);
    DECLARE_THEMED_PROPERTY(SplitPanel, xpf::Color, Background_Hover, xpf::Colors::DodgerBlue, Invalidates::Visuals);
    float m_splitAt = -1;
    bool m_leftIsCollapsed = false;
    bool m_rightIsCollapsed = false;
    Visibility m_visibilityUsedForMeasuring = Visibility::Visible;
    rectf_t m_leftRect;
    rectf_t m_rightRect;
    rectf_t m_splitterRect;
    RenderBatch m_renderSplitter;
    Tween<xpf::Color> m_colorAnimator;

public:
    SplitPanel() : MouseClickTrackerMixIn<Panel>(UIElementType::SplitPanel) { }

    void ComputeWidths(float totalWidth, float& leftWidth, float& rightWidth)
    {
        leftWidth = 0;
        if (m_splitAt == -1 || m_visibilityUsedForMeasuring != m_Visibility)
        {
            m_visibilityUsedForMeasuring = m_Visibility;
            float len_stars = 0;
            float len_pixels = 0;
            for (const auto& sp : m_children)
            {
                const PanelLength* pheight = sp->Get<Panel_Length>();
                if (pheight == nullptr || pheight->type == PanelLength::Auto)
                {
                    len_pixels += sp->GetWidth();
                }
                else if (pheight->type == PanelLength::Star)
                {
                    len_stars += pheight->length;
                }
                else
                {
                    len_stars += pheight->length;
                }
            }

            float len_pixels_removed = (totalWidth - len_pixels);
            if (len_pixels_removed < 0)
                len_pixels_removed = totalWidth;

            float len_per_star = len_stars > 0 ? len_pixels_removed / len_stars : 0;

            const PanelLength* pheight = m_children[0]->Get<Panel_Length>();

            if (pheight == nullptr || pheight->type == PanelLength::Auto)
            {
                leftWidth = m_children[0]->GetWidth();
            }
            else if (pheight->type == PanelLength::Star)
            {
                leftWidth = pheight->length * len_per_star;
            }
            else
            {
                leftWidth = pheight->length;
            }
        }
        else
        {
            leftWidth = m_splitAt;
        }

        {
            float minLen = m_children[0]->GetValueOr<Panel_MinLength>(0.0f);
            float maxLen = m_children[0]->GetValueOr<Panel_MaxLength>(UINT32_MAX);
            if (leftWidth < minLen)
                leftWidth = minLen;
            if (leftWidth > maxLen)
                leftWidth = maxLen;
            
            leftWidth = std::floor(leftWidth);
        }

        rightWidth = totalWidth - leftWidth;

        {
            float minLen = m_children[1]->GetValueOr<Panel_MinLength>(0.0f);
            float maxLen = m_children[1]->GetValueOr<Panel_MaxLength>(UINT32_MAX);
            if (rightWidth < minLen)
            {
                rightWidth = minLen;
                leftWidth = totalWidth - rightWidth;
            }

            if (rightWidth > maxLen)
            {
                rightWidth = maxLen;
                leftWidth = totalWidth - rightWidth;
            }
        }

        float splitterWidth = m_SplitterWidth;
        float halfOfSplitterWidth = splitterWidth * .5f;

        leftWidth = leftWidth - halfOfSplitterWidth;
        rightWidth = totalWidth - (leftWidth + splitterWidth);

        m_splitAt = leftWidth + halfOfSplitterWidth;
    }

    virtual v2_t OnMeasure(v2_t insideSize) override
    {
        if (m_children.size() != 2)
            return insideSize;

        m_leftIsCollapsed = m_children[0]->GetVisibility() == Visibility::Collapsed;
        m_rightIsCollapsed = m_children[1]->GetVisibility() == Visibility::Collapsed;
        float spliterWidth = m_SplitterWidth.Get();

        if (m_Orientation == xpf::Orientation::Horizontal)
        {
            float leftWidth = 0;
            float rightWidth = 0;
            ComputeWidths(insideSize.x, leftWidth, rightWidth);

            if (m_leftIsCollapsed)
            {
                leftWidth = 0;
                rightWidth = insideSize.x;
                spliterWidth = 0;
            }
            else if (m_rightIsCollapsed)
            {
                leftWidth = insideSize.x;
                rightWidth = 0;
                spliterWidth = 0;
            }

            m_children[0]->Measure(v2_t(leftWidth, insideSize.y));
            m_children[1]->Measure(v2_t(rightWidth, insideSize.y));

            m_leftRect = {0,0, leftWidth, insideSize.y};
            m_splitterRect = {m_leftRect.right(), 0, spliterWidth, insideSize.y};
            m_rightRect = {m_splitterRect.right(), 0, rightWidth, insideSize.y};
        }
        else
        {
            float topWidth = 0;
            float bottomWidth = 0;
            ComputeWidths(insideSize.y, topWidth, bottomWidth);

            if (m_leftIsCollapsed)
            {
                topWidth = 0;
                bottomWidth = insideSize.y;
                spliterWidth = 0;
            }
            else if (m_rightIsCollapsed)
            {
                topWidth = insideSize.y;
                bottomWidth = 0;
                spliterWidth = 0;
            }

            m_children[0]->Measure(v2_t(insideSize.x, topWidth));
            m_children[1]->Measure(v2_t(insideSize.x, bottomWidth));

            m_leftRect = {0,0, std::floor(insideSize.x), topWidth};
            m_splitterRect = {0, m_leftRect.bottom(), insideSize.x, spliterWidth};
            m_rightRect = {0, m_splitterRect.bottom(), insideSize.x, bottomWidth};
        }

        return insideSize;
    }

    virtual void OnArrange(v2_t insideSize) override
    {
        if (m_children.size() != 2)
            return;

        m_children[0]->Arrange(m_leftRect);
        m_children[1]->Arrange(m_rightRect);
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        if (m_children.size() == 2)
        {
            if (m_children[0]->GetVisibility() == Visibility::Visible)
                m_children[0]->UpdateVisuals(renderer);

            if (m_children[1]->GetVisibility() == Visibility::Visible)
                m_children[1]->UpdateVisuals(renderer);
        }

        if (!m_leftIsCollapsed && !m_rightIsCollapsed)
        {
            auto r = renderer.CreateCommandBuilder();
            RectangleDescription desc;
            desc.width = m_splitterRect.width;
            desc.height = m_splitterRect.height;
            desc.fillColor = m_SplitterBackground;
            desc.cornerRadius = (m_splitterRect.width > m_splitterRect.height ? m_splitterRect.height : m_splitterRect.width) * .5f;
            r.DrawRectangle(0,0, desc);
            m_renderSplitter = r.Build();
        }
    }

    virtual void OnDraw(IRenderer& renderer) override
    {
        if (m_children.size() != 2)
            return;

        if (m_children[0]->GetVisibility() == Visibility::Visible)
            m_children[0]->Draw(renderer);

        if (m_children[1]->GetVisibility() == Visibility::Visible)
            m_children[1]->Draw(renderer);

        if (!m_leftIsCollapsed && !m_rightIsCollapsed)
        {
            StateManager(renderer);
            auto scope = renderer.TranslateTransfrom(m_splitterRect.x, m_splitterRect.y);
            renderer.EnqueueCommands(m_renderSplitter);
        }
    }

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
            SetSplitterBackground(m_Background_Default);
            return;
        }

        if (m_colorAnimator.IsPlaying())
            SetSplitterBackground(m_colorAnimator.Read());

        v2_t pos = InputService::GetMousePosition();
        auto topleft = renderer.GetCurrentTransform().get_position();
        rectf_t rect = m_splitterRect.move(topleft.x, topleft.y);
        bool isMouseInside = rect.is_inside(pos);
        if (!TrackMouse(isMouseInside))
            return;

        if (m_tracking)
        {
            float prevSplitAt = m_splitAt;
            if (m_Orientation == Orientation::Horizontal)
                m_splitAt = pos.x - topleft.x;
            else
                m_splitAt = pos.y - topleft.y;

            if (prevSplitAt != m_splitAt && m_children.size() == 2)
            {
                m_children[0]->InvalidateLayout();
                m_children[1]->InvalidateLayout();
            }

            InvalidateLayout();
            isMouseInside = true;
        }

        if (isMouseInside)
        {
            if (m_playingFocusLoss)
                m_colorAnimator.Stop();

            m_playingFocusLoss = false;
            m_focusLost = false;

            if (m_colorAnimator.IsPlaying())
            {
                // noop
            }
            else if (m_focusGained)
            {
                SetSplitterBackground(m_Background_Hover);
            }
            else if (!m_playingFocusGain)
            {
                m_colorAnimator.Stop();
                m_playingFocusGain = true;
                m_focusGained = false;
                m_colorAnimator
                    .Clear()
                    .From(m_Background)
                    .To(m_Background_Hover, .2f)
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
                m_colorAnimator.Stop();

            m_playingFocusGain = false;
            m_focusGained = false;

            if (m_colorAnimator.IsPlaying())
            {
                // noop
            }
            else if (m_focusLost)
            {
                SetSplitterBackground(m_Background_Default);
            }
            else if (!m_playingFocusLoss)
            {
                m_colorAnimator.Stop();
                m_playingFocusLoss = true;
                m_focusLost = false;
                m_colorAnimator
                    .Clear()
                    .From(m_SplitterBackground)
                    .To(m_Background_Default, .5f)
                    .Play()
                    .OnComplete([this](bool)
                    {
                        m_playingFocusLoss = false;
                        m_focusLost = true;
                    });
            }
        }
    }
};

} // xpf