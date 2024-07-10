#pragma once
#include <xpf/ui/UIElement.h>

namespace xpf {

class ItemPanel : public UIElement
{
protected:
    std::shared_ptr<UIElement> m_spChild;

public:
    ItemPanel(UIElementType type)
        : UIElement(type)
    {
        SetHorizontalAlignment(HorizontalAlignment::Center);
        SetVerticalAlignment(VerticalAlignment::Center);
    }

    virtual v2_t OnMeasure(v2_t constraint) override
    {
        if (m_spChild != nullptr)
            return m_spChild->Measure(constraint);

        return constraint;
    }

    virtual void OnArrange(v2_t finalSize) override
    {
        if (m_spChild != nullptr)
            m_spChild->Arrange(m_insideRect);
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        if (m_spChild == nullptr) return;

        m_spChild->UpdateVisuals(renderer);
    }

    virtual void OnDraw(IRenderer& renderer) override
    {
        if (m_spChild != nullptr)
            m_spChild->Draw(renderer);
    }
};

} // xpf