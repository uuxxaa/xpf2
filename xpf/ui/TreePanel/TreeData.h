#pragma once
#include <xpf/ui/TreePanel/TreeNode.h>
#include <xpf/ui/TreePanel/TreeParentNode.h>
#include <xpf/ui/TreePanel/TreeLeafNode.h>
#include <vector>
#include <memory>

namespace xpf {

class TreeData
{
protected:
    std::vector<std::shared_ptr<TreeNode>> m_nodes;
    std::vector<TreeNode*> m_visible_nodes;
    v2_t m_bounds;
    float m_height_multiplier = 1.2;
    float m_indent = 20;

public:
    void SetHeightMultiplier(float value) { m_height_multiplier = value; }

    void AddNode(std::shared_ptr<TreeNode>&& node)
    {
        m_nodes.push_back(std::move(node));
        m_visible_nodes.clear();
        m_bounds = v2_t();
    }

    bool DeleteNode(TreeNode* pNode)
    {
        TreeParentNode* pParent = pNode->m_pParent;
        if (pParent != nullptr)
            return pParent->RemoveNode(pNode);

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

    const std::vector<TreeNode*>& GetVisibleNodes() const { return m_visible_nodes; }
    const std::vector<std::shared_ptr<TreeNode>>& GetNodes() const { return m_nodes; }

    v2_t Measure(float width)
    {
        m_visible_nodes.clear();

        m_bounds = v2_t();
        for (const auto& spNode : m_nodes)
        {
            v2_t size = spNode->Measure(width, /*depth:*/ 0, m_height_multiplier, m_visible_nodes);
            if (size.x > m_bounds.x)
                m_bounds.x = size.x;

            m_bounds.y += size.y;
        }

        return m_bounds;
    }

    void UpdateVisuals(IRenderer& renderer)
    {
        for (const auto& spNode : m_nodes)
            spNode->UpdateVisuals(renderer);
    }
};

} // xpf
