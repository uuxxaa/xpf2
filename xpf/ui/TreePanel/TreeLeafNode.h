#pragma once
#include "TreeNode.h"

namespace xpf {

class TreeLeafNode : public TreeNode
{
protected:
    RenderBatch m_renderBatch;

public:
    TreeLeafNode() : TreeNode(TreeNodeType::Leaf) {}
};

} // xpf
