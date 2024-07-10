#pragma once
#include <xpf/ui/UIElement.h>

namespace xpf {

class PanelLength
{
public:
    enum LengthType
    {
        Pixel,
        Auto,
        Star,
    };

    LengthType type = LengthType::Auto;
    float length = 0;
    float minLength = 0;
    float maxLength = std::numeric_limits<float>::max();

    PanelLength() = default;
    PanelLength(float l) : type(LengthType::Pixel), length(l) {}
    PanelLength(float l, LengthType type, float minlength = 0, float maxLength = std::numeric_limits<float>::max())
        : length(l)
        , type(type)
        , minLength(minlength)
        , maxLength(maxLength) {}
    PanelLength(const PanelLength&) = default;
    PanelLength(PanelLength&&) = default;

    PanelLength& operator=(const PanelLength&) = default;
    PanelLength& operator=(PanelLength&&) = default;
};

static const inline PanelLength Panel_OneStar = PanelLength(1, PanelLength::Star);
static const inline PanelLength Panel_TwoStars = PanelLength(2, PanelLength::Star);
static const inline PanelLength Panel_ThreeStars = PanelLength(3, PanelLength::Star);
static const inline PanelLength Panel_Auto = PanelLength(1, PanelLength::Auto);
static const inline PanelLength Panel_Pixel(float len) { return PanelLength(len, PanelLength::Pixel); }
static const inline PanelLength Panel_Star(float len) { return PanelLength(len, PanelLength::Star); }

typedef AttachedProperty<PanelLength, ConstexprHash("panel_length")> Panel_Length;
typedef AttachedProperty<float, ConstexprHash("panel_minlength")> Panel_MinLength;
typedef AttachedProperty<float, ConstexprHash("panel_maxlength")> Panel_MaxLength;

class Panel : public UIElement
{
protected:
    std::vector<std::shared_ptr<UIElement>> m_children;

public:
    Panel(UIElementType type) : UIElement(type) { }

#pragma region child access
    const std::vector<std::shared_ptr<UIElement>>& GetChildren() const { return m_children; }

    Panel& AddChild(const std::shared_ptr<UIElement>& sp, bool invalidateLayout = true)
    {
        AddVisual(sp.get());
        if (m_children.empty())
            SetFirstChild(sp.get());
        else
            m_pLast->SetNextSibling(sp.get());

        UIElement* prevLast = m_pLast;

        SetLastChild(sp.get());
        sp->SetPrevSibling(prevLast);
        sp->SetNextSibling(nullptr);

        m_children.push_back(sp);
        if (invalidateLayout)
            InvalidateLayout();
        return *this;
    }

    Panel& RemoveAll()
    {
        m_pFirst = nullptr;
        m_pLast = nullptr;
        for (auto& sp : m_children)
        {
            sp->SetPrevSibling(nullptr);
            sp->SetNextSibling(nullptr);
        }

        m_children.clear();
        InvalidateLayout();
        OnAllChildrenRemoved();
        return *this;
    }
#pragma endregion

    virtual v2_t OnMeasure(v2_t insideSize) override
    {
        for (const auto& sp : m_children)
            sp->Measure(insideSize);

        return insideSize;
    }

    virtual void OnArrange(v2_t finalSize) override
    {
        rectf_t finalRect {0,0, finalSize.x, finalSize.y};
        for (const auto& sp : m_children)
            sp->Arrange(finalRect);
    }

    virtual void OnAllChildrenRemoved() { }

#pragma region draw
protected:
    virtual void OnDraw(IRenderer& renderer) override
    {
        for (const auto& sp : m_children)
            sp->Draw(renderer);
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        for (const auto& sp : m_children)
            sp->UpdateVisuals(renderer);
    }
#pragma endregion
};

} // xpf