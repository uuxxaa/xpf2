#pragma once
#include "TreeNode.h"
#include <xpf/ui/UIElement.h>

namespace xpf {

class TreeParentNode : public TreeNode
{
protected:
    std::vector<std::shared_ptr<TreeNode>> m_nodes;
    std::shared_ptr<UIElement> m_spExpandedIcon;
    std::function<void(TreeParentNode&, bool collapsing)> m_onExpandingOrCollapsing;
    bool m_isExpanded = true;

public:
    TreeParentNode() : TreeNode(TreeNodeType::Parent) {}

    const std::vector<std::shared_ptr<TreeNode>>& GetNodes() const { return m_nodes; }

    void SetExpandedIcon(const std::shared_ptr<UIElement>& sp) { m_spExpandedIcon = sp; }
    void SetOnExpandingOrCollapsing(std::function<void(TreeParentNode&, bool collapsing)>&& fn) { m_onExpandingOrCollapsing = std::move(fn); }

    virtual bool GetIsExpanded() const override { return m_isExpanded; }

    virtual void SetIsExpanded(bool value) override
    {
        if (value && m_onExpandingOrCollapsing)
            m_onExpandingOrCollapsing(*this, !value);

        m_isExpanded = value;
    }
    
    virtual bool ToggleIsExpanded() override
    {
        SetIsExpanded(!m_isExpanded);
        return m_isExpanded;
    }

    void AddNode(const std::shared_ptr<TreeNode>& spNode)
    {
        spNode->m_pParent = this;
        m_nodes.push_back(spNode);
    }

    bool RemoveNode(TreeNode* pNode)
    {
        for (auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++)
        {
            if (iter->get() == pNode)
            {
                m_nodes.erase(iter);
                return true;
            }
        }
        return false;
    }

    void RemoveAllNodes()
    {
        m_nodes.clear();
    }

    size_t ChildCount()
    {
        return m_nodes.size();
    }

    virtual v2_t Measure(float width, int32_t depth, float heightMultiplier, std::vector<TreeNode*>& visibleNodes) override
    {
        m_iconWidth = 0;
        if (m_isExpanded && m_spExpandedIcon != nullptr)
             m_iconWidth = m_spExpandedIcon->GetWidthProperty().ValueOr(m_height);
        else if (m_spIcon != nullptr)
             m_iconWidth = m_spIcon->GetWidthProperty().ValueOr(m_height);

        width -= m_iconWidth;

        v2_t bounds = TreeNode::Measure(width, depth, heightMultiplier, visibleNodes);
        if (!m_isExpanded) return bounds;

        for (const auto& spNode : m_nodes)
        {
            v2_t childSize = spNode->Measure(width, depth + 1, heightMultiplier, visibleNodes);
            if (childSize.x > bounds.x)
                bounds.x = childSize.x;

            bounds.y += childSize.y;
        }

        return bounds;
    }

    virtual void UpdateVisuals(IRenderer& renderer) override
    {
        TreeNode::UpdateVisuals(renderer);

        if (!m_isExpanded) return;

        for (const auto& spNode : m_nodes)
        {
            spNode->UpdateVisuals(renderer);
        }
    }

    virtual void DrawForeground(IRenderer& renderer, float x, float y, bool hover, bool selected) override
    {
        if (m_iconWidth > 0)
        {
            if (m_isExpanded && m_spExpandedIcon != nullptr)
            {
                float icon_yoffset = (m_height - m_spExpandedIcon->GetHeight()) * .5;
                m_spExpandedIcon->SetX(x + m_indent).SetY(y + icon_yoffset).Draw(renderer);
            }
            else if (!m_isExpanded && m_spIcon != nullptr)
            {
                float icon_yoffset = (m_height - m_spIcon->GetHeight()) * .5;
                m_spIcon->SetX(x + m_indent).SetY(y + icon_yoffset).Draw(renderer);
            }
        }

        return TreeNode::DrawForeground(renderer, x, y, hover, selected);
    }
};

} // xpf
