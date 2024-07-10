#pragma once
#include <xpf/renderer/IRenderer.h>
#include <xpf/renderer/common/Font.h>
#include <xpf/renderer/common/RenderBatchBuilder.h>

#include <memory>
#include <xpf/core/stringex.h>
#include <xpf/core/Color.h>
#include <xpf/core/Rectangle.h>
#include <xpf/math/xpfmath.h>
#include <xpf/renderer/ITexture.h>
#include <xpf/renderer/common/FormattedText.h>
#include <xpf/ui/ThemeEngine.h>

namespace xpf {

enum class TreeNodeType
{
    Leaf,
    Parent,
};

class TreeData;
class TreePanel;
class TreeParentNode;

class TreeNode
{
friend TreeData;
friend TreePanel;
friend TreeParentNode;

public:
    static inline constexpr float m_indentWidth = 20;
protected:
    float m_y = 0;
    int32_t m_depth = 0;
    float m_iconWidth = 0;
    float m_height = 0;
    float m_indent = 0;
    std::shared_ptr<UIElement> m_spIcon;
    TreeParentNode* m_pParent = nullptr;

protected:
    TreeNodeType m_type = TreeNodeType::Leaf;
    ThemedFormattedText m_ft = {"TreeNode_Font"};
    ThemedColor m_foreground = {"TreeNode_Foreground"};
    ThemedColor m_background = {"TreeNode_Background"};

    ThemedColor m_background_selecting = {"TreeNode_Background_Selecting"};
    ThemedColor m_background_selected = {"TreeNode_Background_Selected"};
    ThemedColor m_foreground_selected = {"TreeNode_Foreground_Selected"};
    ThemedColor m_foreground_hover = {"TreeNode_Foreground_Hover"};
    ThemedColor m_background_hover = {"TreeNode_Background_Hover"};

public:
    TreeNode(TreeNodeType type) : m_type(type) { }
    virtual ~TreeNode() = default;

    TreeNodeType GetType() const { return m_type; }
    void SetText(std::string_view value) { m_ft.SetText(value); }
    const std::string& GetText() const { return m_ft.GetText(); }

    void SetIcon(const std::shared_ptr<UIElement>& sp) { m_spIcon = sp; }

    void SetThemeId(std::string_view themeId) { m_ft.SetThemeId(themeId); }

    virtual void SetIsExpanded(bool value) {}
    virtual bool GetIsExpanded() const { return true; }
    virtual bool ToggleIsExpanded() { return false; };

    virtual v2_t Measure(float width, int32_t depth, float heightMultiplier, std::vector<TreeNode*>& visibleNodes)
    {
        m_iconWidth = 0;
        if (m_spIcon != nullptr)
            m_iconWidth = m_spIcon->GetWidthProperty().ValueOr(m_height);

        width -= m_iconWidth;

        m_depth = depth;
        m_indent = m_depth * m_indentWidth;
        visibleNodes.push_back(this);

        m_ft.Get().SetMaxWidth(width - m_indent);
        m_ft.Get().BuildGeometry();
        v2_t bounds = m_ft.Get().GetBounds();
        bounds.x += m_indent;
        bounds.y = xpf::math::round_up(bounds.y * heightMultiplier);
        m_height = bounds.y;

        return bounds;
    }

    virtual void UpdateVisuals(IRenderer& renderer)
    {
        if (m_spIcon != nullptr)
            m_spIcon->UpdateVisuals(renderer);

        m_ft.Get().BuildGeometry();
    }

    virtual void DrawForeground(IRenderer& renderer, float x, float y, bool hover, bool selected)
    {
        if (m_type == TreeNodeType::Leaf && m_iconWidth > 0)
        {
            if (m_spIcon != nullptr)
            {
                float icon_yoffset = (m_height - m_spIcon->GetHeight()) * .5;
                m_spIcon->SetX(round(x + m_indent)).SetY(y + icon_yoffset).Draw(renderer);
            }
        }

        xpf::Color foreground = m_foreground.Get();
        if (hover)
            foreground = xpf::math::midpoint(foreground, m_foreground_hover.Get());

        if (selected)
            foreground = xpf::math::midpoint(foreground, m_foreground_selected.Get());

        float text_yoffset = y + (m_height - m_ft.Get().GetHeight()) * .5;
        renderer.DrawText(x + m_indent + m_iconWidth, text_yoffset, m_ft.Get(), m_foreground.Get());
    }

    virtual bool HitTest(v2_t pos) const
    {
        return (m_y < pos.y && pos.y < m_y + m_height);
    }
};

} // xpf
