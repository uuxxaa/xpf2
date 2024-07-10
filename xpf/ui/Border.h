#pragma once
#include <xpf/ui/UIElement.h>

namespace xpf {

class Border : public UIElement
{
protected:
    std::shared_ptr<UIElement> m_spChild;
    v2_t m_childOutsideMeasurement;

public:
    Border() : UIElement(UIElementType::Border)
    {
        m_clippingEnabled = true;
    }

    Border& SetChild(const std::shared_ptr<UIElement>& sp)
    {
        m_spChild = sp;
        m_pFirst = m_pLast = sp.get();
        if (sp != nullptr)
        {
            sp->SetNextSibling(nullptr);
            sp->SetPrevSibling(nullptr);
            AddVisual(sp.get());
        }
        InvalidateLayout();
        return *this;
    }
    UIElement& GetChild() { return *m_spChild; }

#pragma region layout
public:
    virtual v2_t OnMeasure(v2_t insideSize) override
    {
        if (m_HorizontalAlignment != HorizontalAlignment::Stretch)
            insideSize.x = m_insideRect.width;

        if (m_VerticalAlignment != VerticalAlignment::Stretch)
            insideSize.y = m_insideRect.height;

        if (m_spChild == nullptr)
            return insideSize;

        m_spChild->Measure(insideSize);
        m_childOutsideMeasurement = m_spChild->GetDesiredSize();

        if (m_HorizontalAlignment == HorizontalAlignment::Stretch)
            m_childOutsideMeasurement.x = insideSize.x;

        if (m_VerticalAlignment == VerticalAlignment::Stretch)
            m_childOutsideMeasurement.y = insideSize.y;

        return v2_t(
            std::max(m_childOutsideMeasurement.w + m_Padding.Get().left_right() + m_BorderThickness.Get().left_right(), 0.0f),
            std::max(m_childOutsideMeasurement.h + m_Padding.Get().top_bottom() + m_BorderThickness.Get().top_bottom(), 0.0f));
    }

    virtual void OnArrange(v2_t insideSize) override
    {
        if (m_spChild != nullptr)
            m_spChild->Arrange(m_insideRect);
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        if (m_spChild != nullptr)
            m_spChild->UpdateVisuals(renderer);
    }
#pragma endregion

#pragma region draw
protected:
    virtual void OnDraw(IRenderer& renderer) override
    {
        if (m_spChild != nullptr)
            m_spChild->Draw(renderer);
    }
#pragma endregion
};

} // xpf
