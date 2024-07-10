#pragma once
#include <xpf/core/Color.h>
#include <xpf/core/CornerRadius.h>
#include <xpf/core/Rectangle.h>
#include <xpf/core/Time.h>
#include <xpf/core/Thickness.h>
#include <xpf/ui/AttachedProperty.h>
#include <xpf/ui/UIProperty.h>
#include <xpf/ui/ThemeEngine.h>
#include <xpf/renderer/RenderCommand.h>
#include <xpf/renderer/IRenderer.h>
#include <xpf/renderer/common/RenderBatchBuilder.h>
#include <xpf/windows/InputService.h>

namespace xpf  {

enum class UIElementType
{
    UIElement,
    Border,
    Button,
    GridPanel,
    ImagePanel,
    Scrollbar,
    SplitPanel,
    StackPanel,
    SwitchBox,
    TextBlock,
    TextBox,
    TreePanel,
};

enum class HorizontalAlignment
{
    Left,
    Center,
    Right,
    Stretch,
};

enum class VerticalAlignment 
{
    Top,
    Center,
    Bottom,
    Stretch,
};

static inline xpf::string_viewex to_string(VerticalAlignment v)
{
    switch(v) {
        case VerticalAlignment::Top: return "Top";
        case VerticalAlignment::Center: return "Center";
        case VerticalAlignment::Bottom: return "Bottom";
        case VerticalAlignment::Stretch: return "Stretch";
    }
}

enum class Orientation
{
    Vertical,
    Horizontal,
};

enum class Visibility
{
    Visible,   // Display the element.
    Hidden,    // Do not display the element, but reserve space for the element in layout.
    Collapsed, // Do not display the element, and do not reserve space for it in layout.
};

class UIElementBase {};
class UIElement : public ObjectWithAttachedProperties<UIElementBase>
{
private:
    RenderBatch m_background;

protected:
    const UIElementType m_elementType;
    UIElement* m_pParent = nullptr;
    UIElement* m_pFirst = nullptr;
    UIElement* m_pLast = nullptr;
    UIElement* m_pNext = nullptr;
    UIElement* m_pPrev = nullptr;
    bool m_layoutInvalidated = true;
    bool m_visualsInvalidated = true;
    bool m_clippingEnabled = false;
    bool m_pixel_perfect = false;

#pragma region properties
protected:
    DECLARE_PROPERTY(UIElement, std::string, Name, std::string(), Invalidates::None);
    DECLARE_PROPERTY(UIElement, float, X, 0, Invalidates::None);
    DECLARE_PROPERTY(UIElement, float, Y, 0, Invalidates::None);
    DECLARE_PROPERTY(UIElement, float, Width, 0, Invalidates::ParentLayout);
    DECLARE_PROPERTY(UIElement, float, Height, 0, Invalidates::ParentLayout);
    DECLARE_PROPERTY(UIElement, float, MinWidth, 0, Invalidates::ParentLayout);
    DECLARE_PROPERTY(UIElement, float, MinHeight, 0, Invalidates::ParentLayout);
    DECLARE_PROPERTY(UIElement, float, MaxWidth, std::numeric_limits<float>::max(), Invalidates::ParentLayout);
    DECLARE_PROPERTY(UIElement, float, MaxHeight, std::numeric_limits<float>::max(), Invalidates::ParentLayout);
    DECLARE_PROPERTY(UIElement, xpf::Color, Foreground, xpf::Colors::Transparent, Invalidates::Visuals);
    DECLARE_PROPERTY(UIElement, xpf::Color, Background, xpf::Colors::Transparent, Invalidates::Visuals);
    DECLARE_PROPERTY(UIElement, xpf::Color, Border, xpf::Colors::Transparent, Invalidates::Visuals);
    DECLARE_PROPERTY(UIElement, xpf::CornerRadius, CornerRadius, 0, Invalidates::Visuals);
    DECLARE_PROPERTY(UIElement, xpf::Thickness, Margin, 0, Invalidates::SelfLayout);
    DECLARE_PROPERTY(UIElement, xpf::Thickness, BorderThickness, 0, Invalidates::SelfLayout);
    DECLARE_PROPERTY(UIElement, xpf::Thickness, Padding, 0, Invalidates::SelfLayout);
    DECLARE_PROPERTY(UIElement, HorizontalAlignment, HorizontalAlignment, HorizontalAlignment::Stretch, Invalidates::SelfLayout);
    DECLARE_PROPERTY(UIElement, VerticalAlignment, VerticalAlignment, VerticalAlignment::Stretch, Invalidates::SelfLayout);
    DECLARE_PROPERTY(UIElement, float, VerticalScrollPosition, 0, Invalidates::SelfLayout);
    DECLARE_PROPERTY(UIElement, float, HorizontalScrollPosition, 0, Invalidates::SelfLayout);
    DECLARE_PROPERTY(UIElement, bool, IsHitTestVisible, true, Invalidates::None);
    DECLARE_PROPERTY(UIElement, Visibility, Visibility, Visibility::Visible, Invalidates::ParentLayout);
    DECLARE_PROPERTY(UIElement, float, RotationAngle, 0, Invalidates::Visuals);
    DECLARE_PROPERTY(UIElement, v2_t, Translation, 0, Invalidates::Visuals);
    DECLARE_PROPERTY(UIElement, bool, ShowDebugView, false, Invalidates::Visuals);
    ThemedColor m_focusBorder = {"Focus_Border"};
    std::optional<v2_t> m_optUnclippedDesiredSizeField;
#pragma endregion

#pragma region layout
private:
friend struct MinMax;
    struct MinMax
    {
        float minWidth;
        float maxWidth;
        float minHeight;
        float maxHeight;

        MinMax(const UIElement& e)
        {
            maxHeight = e.m_MaxHeight;
            minHeight = e.m_MinHeight;

            float height  = e.m_Height.IsSet() ? e.m_Height : std::numeric_limits<float>::max();
            maxHeight = std::max<float>(std::min<float>(height, maxHeight), minHeight);

            height = (e.m_Height.IsSet() ? e.m_Height : 0.0f);
            minHeight = std::max<float>(std::min<float>(maxHeight, height), minHeight);

            maxWidth = e.m_MaxWidth;
            minWidth = e.m_MinWidth;

            float width = e.m_Width.IsSet() ? e.m_Width : std::numeric_limits<float>::max();
            maxWidth = std::max<float>(std::min<float>(width, maxWidth), minWidth);

            width = (e.m_Width.IsSet() ? e.m_Width : 0.0f);
            minWidth = std::max<float>(std::min<float>(maxWidth, width), minWidth);
        }
    };

protected:
    //
    //    margin   border   paddding inside   inside      padding  border   margin
    //    |        |        |        |             |      |        |        |
    //                               ---- inside ---
    //                      ------padding-----------------
    //             ----------border---------------------------------
    //    -------------------------margin------------------------------------

public:
    void InvalidateParentLayout() { if (m_pParent != nullptr) { m_pParent->InvalidateLayout(); } InvalidateLayout(); }
    void InvalidateLayout() { m_layoutInvalidated = m_visualsInvalidated = true;  }
    void InvalidateVisuals() { m_visualsInvalidated = true; }
    const rectf_t& GetActualRect() const { return m_marginRect; }
    v2_t GetDesiredSize() const { return m_desired_size; }

protected:
    rectf_t m_marginRect;
    rectf_t m_borderRect;
    rectf_t m_paddingRect;
    rectf_t m_insideRect;

    rectf_t m_actualRect;
    v2_t m_assigned_size;
    v2_t m_desired_size;
    v2_t m_measure_outside_constraint;
    rectf_t m_finalRect;
    static inline bool m_rootArrangeSizeNeedsComputed = true;
    bool m_needsClipBounds = false;
    bool m_bypassLayoutPolicies = false;

private:
    void Layout(IRenderer& renderer)
    {
        if (m_pParent == nullptr && m_rootArrangeSizeNeedsComputed)
        {
            m_rootArrangeSizeNeedsComputed = false;
            m_measure_outside_constraint = v2_t{
                m_Width.ValueOr(800) - GetLeftRightThickness(),
                m_Height.ValueOr(480) - GetTopBottomThickness() };
        }

        if (m_layoutInvalidated)
        {
            Measure(m_measure_outside_constraint);
            Arrange(m_finalRect);
        }

        if (m_visualsInvalidated)
        {
            UpdateVisuals(renderer);
        }
    }

protected:
    float GetLeftRightThickness() const
    {
        return
            m_Padding.Get().left_right() +
            m_BorderThickness.Get().left_right() +
            m_Margin.Get().left_right();
    }

    float GetTopBottomThickness() const
    {
        return
            m_Padding.Get().top_bottom() +
            m_BorderThickness.Get().top_bottom() +
            m_Margin.Get().top_bottom();
    }

protected:
    v2_t ComputeAlignmentOffset(v2_t clientSize, v2_t inkSize)
    {
        v2_t offset;

        HorizontalAlignment ha = m_HorizontalAlignment;
        VerticalAlignment va = m_VerticalAlignment;

        //this is to degenerate Stretch to Top-Left in case when clipping is about to occur
        //if we need it to be Center instead, simply remove these 2 ifs
        if (ha == HorizontalAlignment::Stretch && inkSize.w > clientSize.w)
            ha = HorizontalAlignment::Left;

        if (va == VerticalAlignment::Stretch && inkSize.h > clientSize.h)
            va = VerticalAlignment::Top;
        //end of degeneration of Stretch to Top-Left

        if (ha == HorizontalAlignment::Center) // || ha == HorizontalAlignment::Stretch)
            offset.x = (clientSize.w - inkSize.w) * 0.5f;
        else if (ha == HorizontalAlignment::Right)
            offset.x = clientSize.w - inkSize.w;

        if (va == VerticalAlignment::Center) // || va == VerticalAlignment::Stretch)
            offset.y = (clientSize.h - inkSize.h) * 0.5;
        else if (va == VerticalAlignment::Bottom)
            offset.y = clientSize.h - inkSize.h;

        return offset;
    }

public:
    // outsideSize includes margin, border, padding thicknesses
    // computes desired size including margin, border, padding thicknesses
    v2_t Measure(v2_t outsideSize)
    {
        m_measure_outside_constraint = outsideSize;
        if (m_bypassLayoutPolicies)
            return OnMeasure(outsideSize);

        const Thickness& margin = m_Margin;
        v2_t availableSize(
            std::max(outsideSize.w - margin.left_right(), 0.0f),
            std::max(outsideSize.h - margin.top_bottom(), 0.0f));

        MinMax mm(*this);
        availableSize.w = std::max<float>(mm.minWidth, std::min<float>(availableSize.w, mm.maxWidth));
        availableSize.h = std::max<float>(mm.minHeight, std::min<float>(availableSize.h, mm.maxHeight));

        v2_t insideSize(
            availableSize.x - m_Padding.Get().left_right() - m_BorderThickness.Get().left_right(),
            availableSize.y - m_Padding.Get().top_bottom() - m_BorderThickness.Get().top_bottom());

        v2_t desiredSize = OnMeasure(insideSize);
        desiredSize.x += m_Padding.Get().left_right() + m_BorderThickness.Get().left_right();
        desiredSize.y += m_Padding.Get().top_bottom() + m_BorderThickness.Get().top_bottom();

        //  maximize desiredSize with user provided min size
        desiredSize = v2_t(
            std::max<float>(desiredSize.w, mm.minWidth),
            std::max<float>(desiredSize.h, mm.minHeight));

        //here is the "true minimum" desired size - the one that is
        //for sure enough for the control to render its content.
        v2_t unclippedDesiredSize = desiredSize;
        bool clipped = false;

        // User-specified max size starts to "clip" the control here.
        //Starting from this point desiredSize could be smaller then actually
        //needed to render the whole control
        if (desiredSize.w > mm.maxWidth)
        {
            desiredSize.w = mm.maxWidth;
            clipped = true;
        }

        if (desiredSize.h > mm.maxHeight)
        {
            desiredSize.h = mm.maxHeight;
            clipped = true;
        }

        //  because of negative margins, clipped desired size may be negative.
        //  need to keep it as doubles for that reason and maximize with 0 at the
        //  very last point - before returning desired size to the parent.
        float clippedDesiredWidth = desiredSize.w + margin.left_right();
        float clippedDesiredHeight = desiredSize.h + margin.top_bottom();

        // In overconstrained scenario, parent wins and measured size of the child,
        // including any sizes set or computed, can not be larger then
        // available size. We will clip the guy later.
        if (clippedDesiredWidth > availableSize.w)
        {
            clippedDesiredWidth = availableSize.w;
            clipped = true;
        }

        if (clippedDesiredHeight > availableSize.h)
        {
            clippedDesiredHeight = availableSize.h;
            clipped = true;
        }

        if (clipped || clippedDesiredWidth < 0 || clippedDesiredHeight < 0)
            m_optUnclippedDesiredSizeField = unclippedDesiredSize;
        else
            m_optUnclippedDesiredSizeField = std::nullopt;

        m_desired_size = v2_t(std::max(0.0f, clippedDesiredWidth), std::max(0.0f, clippedDesiredHeight));
        desiredSize.x += margin.left_right();
        desiredSize.y += margin.top_bottom();
        m_finalRect = {m_X, m_Y, m_desired_size.x, m_desired_size.y};
        return desiredSize;
    }

    // finalRect includes, margin, border and padding
    void Arrange(rectf_t finalRect)
    {
        m_visualsInvalidated = true;
        if (m_bypassLayoutPolicies)
        {
            // Size oldRenderSize = RenderSize;
            // Size inkSize = ArrangeOverride(finalRect.Size);
            // RenderSize = inkSize;
            // SetLayoutOffset(new Vector(finalRect.X, finalRect.Y), oldRenderSize);
            OnArrange(finalRect.size());
        }
        else
        {
            // This is computed on every Arrange.
            // Depending on LayoutConstrained, actual clip may apply or not
            m_needsClipBounds = false;
            v2_t arrangeSize = finalRect.size();

            const Thickness& margin = m_Margin;
            float marginWidth = margin.left_right();
            float marginHeight = margin.top_bottom();

            arrangeSize.w = std::max(0.0f, arrangeSize.w - marginWidth);
            arrangeSize.h = std::max(0.0f, arrangeSize.h - marginHeight);

            v2_t unclippedDesiredSize;
            if (m_optUnclippedDesiredSizeField.has_value())
            {
                unclippedDesiredSize = m_optUnclippedDesiredSizeField.value();
            }
            else
            {
                unclippedDesiredSize.x = std::max(0.0f, m_desired_size.w - margin.left_right());
                unclippedDesiredSize.y = std::max(0.0f, m_desired_size.h - margin.top_bottom());
            }

            if (arrangeSize.w < unclippedDesiredSize.w)
            {
                m_needsClipBounds = true;
                arrangeSize.w = unclippedDesiredSize.w;
            }

            if (arrangeSize.h < unclippedDesiredSize.h)
            {
                m_needsClipBounds = true;
                arrangeSize.h = unclippedDesiredSize.h;
            }

            // Alignment==Stretch --> arrange at the slot size minus margins
            // Alignment!=Stretch --> arrange at the unclippedDesiredSize
            if (m_HorizontalAlignment != HorizontalAlignment::Stretch)
                arrangeSize.w = unclippedDesiredSize.w;

            if (m_VerticalAlignment != VerticalAlignment::Stretch)
                arrangeSize.h = unclippedDesiredSize.h;

            MinMax mm(*this);

            //we have to choose max between UnclippedDesiredSize and Max here, because
            //otherwise setting of max property could cause arrange at less then unclippedDS.
            //Clipping by Max is needed to limit stretch here
            float effectiveMaxWidth = std::max(unclippedDesiredSize.w, mm.maxWidth);
            if (effectiveMaxWidth < arrangeSize.w)
            {
                m_needsClipBounds = true;
                arrangeSize.w = effectiveMaxWidth;
            }

            float effectiveMaxHeight = std::max(unclippedDesiredSize.h, mm.maxHeight);
            if (effectiveMaxHeight < arrangeSize.h)
            {
                m_needsClipBounds = true;
                arrangeSize.h = effectiveMaxHeight;
            }

            v2_t innerSize = arrangeSize;

            // clippedSize differs from innerSize only what MaxWidth/Height explicitly clip the
            // otherwise good arrangement. For ex, DS<clientSize but DS>MaxWidth - in this
            // case we should initiate clip at MaxWidth and only show Top-Left portion
            // of the element limited by Max properties. It is Top-left because in case when we
            // are clipped by container we also degrade to Top-Left, so we are consistent.
            v2_t clippedSize(
                std::min(innerSize.w, mm.maxWidth),
                std::min(innerSize.h, mm.maxHeight));

            //remember we have to clip if Max properties limit the inkSize
            m_needsClipBounds |=
                (clippedSize.w < innerSize.w) ||
                (clippedSize.h < innerSize.h);

            //Note that inkSize now can be bigger then layoutSlotSize-margin (because of layout
            //squeeze by the parent or LayoutConstrained=true, which clips desired size in Measure).

            // The client size is the size of layout slot decreased by margins.
            // This is the "window" through which we see the content of the child.
            // Alignments position ink of the child in this "window".
            // Max with 0 is neccessary because layout slot may be smaller then unclipped desired size.
            v2_t clientSize(
                std::max(0.0f, finalRect.w),
                std::max(0.0f, finalRect.h));

            // remember we have to clip if clientSize limits the inkSize
            m_needsClipBounds |=
                    (clientSize.w < clippedSize.w) ||
                    (clientSize.h < clippedSize.h);

            v2_t offset = ComputeAlignmentOffset(clientSize, clippedSize);

            m_marginRect = {
                0, 0,
                std::min(clientSize.w, clippedSize.w) + marginWidth,
                std::min(clientSize.h, clippedSize.h) + marginHeight};
            m_borderRect = m_marginRect.shrink(m_Margin);
            m_paddingRect = m_borderRect.shrink(m_BorderThickness);
            m_insideRect = m_paddingRect.shrink(m_Padding);
            
            finalRect.x += xpf::math::clamp_min(0, offset.x - margin.l);
            finalRect.y += xpf::math::clamp_min(0, offset.y - margin.t);
            v2_t size = m_insideRect.size();
            if (m_pixel_perfect)
            {
                finalRect.x = xpf::math::round_down(finalRect.x);
                finalRect.y = xpf::math::round_down(finalRect.y);
                size = size.round_up();
            }

            SetX(finalRect.x);
            SetY(finalRect.y);
            OnArrange(size);
        }

        m_on_resize(*this);

        m_layoutInvalidated = false;
    }

    void UpdateVisuals(IRenderer& renderer)
    {
        if (!m_on_update_visuals(*this, renderer))
            return;

        OnUpdateVisuals(renderer);

        if (!m_Background.Get().is_transparent() || (m_Border.Get().is_transparent() && !m_BorderThickness.Get().is_zero_or_negative()))
        {
            m_background = BuildBackground(renderer);
        }
        else
        {
            m_background.Clear();
        }

        m_visualsInvalidated = false;
    }

    virtual RenderBatch BuildBackground(IRenderer& renderer)
    {
        auto r = renderer.CreateCommandBuilder();
        r.DrawRectangle(
            m_borderRect.x,
            m_borderRect.y, {
            m_borderRect.width,
            m_borderRect.height,
            HorizontalOrientation::Left,
            VerticalOrientation::Top,
            m_Background,
            m_Border,
            m_BorderThickness,
            m_CornerRadius});

        if (m_ShowDebugView)
        {
            r.DrawRectangle(
                m_marginRect.x,
                m_marginRect.y, {
                m_marginRect.width,
                m_marginRect.height,
                HorizontalOrientation::Left,
                VerticalOrientation::Top,
                xpf::Colors::Transparent,
                xpf::Colors::Gray,
                1,
                0});
        }

        return r.Build();
    }

protected:
    // constraint is without margin, border or padding.
    // returns inner desired size, without margin, border or padding.
    virtual v2_t OnMeasure(v2_t constraint) { return {0,0}; }

    // insideSize is without margin, border or padding, computed from m_insideRect.
    virtual void OnArrange(v2_t insideSize) { }
    virtual void OnUpdateVisuals(IRenderer& renderer) {}
#pragma endregion

#pragma region visual elements
public:
    void AddVisual(UIElement* pElement) { pElement->m_pParent = this; }
#pragma endregion

#pragma region draw
public:
    void Draw(IRenderer& renderer)
    {
        Layout(renderer);

        m4_t m = m4_t::translation_matrix(m_X, m_Y);
        if (m_RotationAngle.IsSet())
        {
            float centerx = m_insideRect.x - m_insideRect.w *.5;
            float centery = m_insideRect.y - m_insideRect.h *.5;
            m *= m4_t::translation_matrix(
                -centerx,
                -centery);
            m *= m4_t::rotation_matrix_z(m_RotationAngle);
            m *= m4_t::translation_matrix(
                centerx,
                centery);
        }

        if (m_Translation.IsSet())
            m *= m4_t::translation_matrix(m_Translation.Get().x, m_Translation.Get().y);

        auto scope =  renderer.Transform(m);

        ProcessMouseInput(renderer);

        // draw background
        renderer.EnqueueCommands(m_background);

        // draw foreground
        if (m_clippingEnabled || m_needsClipBounds)
        {
            auto topleft = renderer.GetCurrentTransform().get_position();
            auto clipScope = renderer.Clip({
                uint32_t(m_insideRect.x + topleft.x), uint32_t(m_insideRect.y + topleft.y),
                uint32_t(m_insideRect.w), uint32_t(m_insideRect.h)});

            OnDraw(renderer);
            m_on_draw(*this, renderer);
        }
        else
        {
            OnDraw(renderer);
            m_on_draw(*this, renderer);
        }

         DrawFocus(renderer);
    }

    virtual void DrawFocus(IRenderer& renderer)
    {
        if (IsInFocus())
            DrawFocusRectangle(renderer, m_paddingRect, m_CornerRadius);
    }

    void DrawFocusRectangle(IRenderer& renderer, rectf_t rect, CornerRadius cornerRadius)
    {
        RectangleDescription desc;
        desc.fillColor = xpf::Colors::Transparent;
        desc.borderColor = m_focusBorder.Get();
        desc.borderThickness = 1;
        desc.cornerRadius = m_CornerRadius;
        desc.width = xpf::math::round_up(rect.w);
        desc.height = xpf::math::round_up(rect.h);
        desc.borderType = RectangleDescription::Dot;
        renderer.DrawRectangle(xpf::math::round_down(rect.x), xpf::math::round_down(rect.y), desc);
    }

protected:
    std::function<bool(UIElement&, IRenderer&)> m_on_draw = [](UIElement&, IRenderer&) { return true; };
    std::function<bool(UIElement&, IRenderer&)> m_on_update_visuals = [](UIElement&, IRenderer&) { return true; };
    std::function<void(UIElement&)> m_on_resize = [](UIElement&) { };

    virtual void OnDraw(IRenderer& renderer) { }
#pragma endregion

#pragma region events
public:
    UIElement& SetOnMouse(std::function<void(UIElement&, bool)>&& fn) { m_on_mouse = std::move(fn); return *this; }
    UIElement& SetOnMouseEnter(std::function<void(UIElement&)>&& fn) { m_on_mouse_enter = std::move(fn); return *this; }
    UIElement& SetOnMouseLeave(std::function<void(UIElement&)>&& fn) { m_on_mouse_leave = std::move(fn); return *this; }
    UIElement& SetOnDraw(std::function<bool(UIElement&, IRenderer&)>&& fn) { m_on_draw = std::move(fn); return *this; }
    UIElement& SetOnUpdateVisuals(std::function<bool(UIElement&, IRenderer&)>&& fn) { m_on_update_visuals = std::move(fn); return *this; }
    UIElement& SetOnResize(std::function<void(UIElement&)>&& fn) { m_on_resize = std::move(fn); return *this; }
#pragma endregion

#pragma region focus
private:
    static inline UIElement* m_pFocusedElement = nullptr;
protected:
    bool m_acceptsFocus = false;

public:
    void Focus() { TryApplyFocus(false); }
    bool IsInFocus() const { return m_pFocusedElement == this; }

protected:
    virtual void OnFocusGain() { }

    bool TryApplyFocus(bool reverse)
    {
        if (m_acceptsFocus)
        {
            OnFocusGain();
            InputService::Focus();
            m_pFocusedElement = this;
            return true;
        }

        UIElement* pElement = nullptr;
        for (;;)
        {
            pElement = reverse
                ? (pElement == nullptr ? m_pLast : pElement->m_pPrev)
                : (pElement == nullptr ? m_pFirst : pElement->m_pNext);
            if (pElement == nullptr)
                return false;
            if (pElement->TryApplyFocus(reverse))
                return true;
        }

        return false;
    }

    bool SetFocusToNextElement(UIElement* pElement, bool reverse)
    {
        for (;;)
        {
            pElement = reverse
                ? (pElement == nullptr ? m_pLast : pElement->m_pPrev)
                : (pElement == nullptr ? m_pFirst : pElement->m_pNext);
            if (pElement == nullptr)
            {
                // if we could not find next focusable element under this element
                // escalate it to the parent
                if (m_pParent != nullptr)
                    return m_pParent->SetFocusToNextElement(this, reverse);
                return false;
            }
            else
            {
                if (pElement->TryApplyFocus(reverse))
                    return true;
            }
        }
    }
#pragma endregion#pragma endregion

#pragma region mouse input
private:
    bool m_is_mouse_inside = false;
    std::function<void(UIElement&, bool)> m_on_mouse = [](UIElement&, bool) { };
    std::function<void(UIElement&)> m_on_mouse_enter = [](UIElement&) { };
    std::function<void(UIElement&)> m_on_mouse_leave = [](UIElement&) { };
    static inline UIElement* m_pMouseCapturedBy = nullptr;

public:
    v2_t GetMousePosition(IRenderer& r) const { return InputService::GetMousePosition() - r.GetCurrentTransform().get_position().xy(); }
    bool IsMouseInside() const { return m_is_mouse_inside; }

    virtual void CaptureMouse() { m_pMouseCapturedBy = this; }
    virtual void ReleaseMouse() { m_pMouseCapturedBy = nullptr; }
    virtual bool IsMouseCaptured() { return m_pMouseCapturedBy != nullptr && m_pMouseCapturedBy != this; }

    virtual void OnMouse(bool is_inside) { m_on_mouse(*this, is_inside); }
    virtual void OnMouseEnter() { m_on_mouse_enter(*this); }
    virtual void OnMouseLeave() { m_on_mouse_leave(*this); }

public:
    void SetFirstChild(UIElement* pChild) { m_pFirst = pChild; }
    UIElement* GetFirstChild() { return m_pFirst; }
    void SetLastChild(UIElement* pChild) { m_pLast = pChild; }

    void SetNextSibling(UIElement* pNext) { m_pNext = pNext; }
    UIElement* GetNextSibling() const { return m_pNext; }
    void SetPrevSibling(UIElement* pPrev) { m_pPrev = pPrev; }
    UIElement* GetPrevSibling() const { return m_pPrev; }

protected:
    enum class KeyboardState
    {
        WaitingForKeyDown,
        WaitingForKeyUp,
    };
    static inline KeyboardState m_keyboardState = KeyboardState::WaitingForKeyDown;
    void ProcessMouseInput(IRenderer& renderer)
    {
        if (m_pParent == nullptr)
        {
            if (m_pFocusedElement != nullptr && s_tab.IsKeyPressed())
            {
                bool reverse = (InputService::IsKeyDown(KeyCode::LeftShift) ||
                    InputService::IsKeyDown(KeyCode::RightShift));

                auto pParent = m_pFocusedElement->m_pParent;
                if (pParent != nullptr)
                {
                    if (!pParent->SetFocusToNextElement(m_pFocusedElement, reverse))
                        SetFocusToNextElement(nullptr, reverse);
                }
            }
        }

        if (m_on_mouse == nullptr) return;

        const bool isInside = GetIsHitTestVisible() && m_borderRect.is_inside(GetMousePosition(renderer));
        if (isInside)
        {
            if (!m_is_mouse_inside)
                OnMouseEnter();
            m_is_mouse_inside = true;
        }
        else if (m_is_mouse_inside)
        {
            OnMouseLeave();
            m_is_mouse_inside = false;
        }

        OnMouse(isInside);
    }

protected:
    struct KeyInfo
    {
        KeyCode m_keycode = KeyCode::None;
        time_t m_time_KeyDown = std::numeric_limits<time_t>::max();
        time_t m_waitBeforeRepat = 0;

        KeyInfo(KeyCode keycode) : m_keycode(keycode) {}

        bool IsKeyPressed()
        {
            if (!InputService::IsKeyDown(m_keycode))
            {
                m_waitBeforeRepat = .125;
                m_time_KeyDown = 0;
                return false;
            }

            if ((Time::GetTime() - m_time_KeyDown) > m_waitBeforeRepat)
            {
                m_time_KeyDown = Time::GetTime();
                m_waitBeforeRepat = .125;
                return true;
            }
            return false;
        }
    };

    static inline KeyInfo s_tab = {KeyCode::Tab};
    static inline KeyInfo s_space = {KeyCode::Space};
    static inline KeyInfo s_backspace = {KeyCode::Backspace};
    static inline KeyInfo s_a = {KeyCode::A};
    static inline KeyInfo s_c = {KeyCode::C};
    static inline KeyInfo s_v = {KeyCode::V};
    static inline KeyInfo s_x = {KeyCode::X};
    static inline KeyInfo s_z = {KeyCode::Z};
    static inline KeyInfo s_up = {KeyCode::Up};
    static inline KeyInfo s_down = {KeyCode::Down};
    static inline KeyInfo s_left = {KeyCode::Left};
    static inline KeyInfo s_right = {KeyCode::Right};
    static inline KeyInfo s_enter = {KeyCode::Enter};
#pragma endregion

public:
    UIElement(UIElementType type)
        : m_elementType(type)
    {}

    UIElementType GetType() const { return m_elementType; }

    template<typename T> T* As() { return static_cast<T*>(this); }
};

template<typename T>
void UIProperty<T>::Set(const T& f) {
    if (!m_isReadOnly)
        return;

    T oldval = m_value;
    m_value = f;

    if (m_value != oldval || !m_isSet) {
        m_isSet = true;
        m_onChanged(m_pOwner, m_value);
        if (m_pOwner != nullptr) {
            switch (m_invalidates) {
                case Invalidates::None:
                    break;
                case Invalidates::SelfLayout:
                    m_pOwner->InvalidateLayout();
                    break;
                case Invalidates::ParentLayout:
                    m_pOwner->InvalidateParentLayout();
                    break;
                case Invalidates::Visuals:
                    m_pOwner->InvalidateVisuals();
                    break;
            }
        }
    }
}

} // xpf
