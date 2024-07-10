#pragma once
#include <xpf/ui/UIElement.h>
#include <xpf/ui/ScrollPanelMixIn.h>
#include <xpf/ui/TreePanel/TreeData.h>
#include <xpf/core/Log.h>
#include <xpf/windows/InputService.h>
#include <unordered_map>

namespace xpf {

enum class ExpandTrigger
{
    SingleClick,
    DoubleClick,
};

class TreePanel : public ScrollPanelMixIn<UIElement>
{
protected:
    std::shared_ptr<TreeData> m_spData = std::make_shared<TreeData>();
    std::vector<TreeNode*> m_drawnNodes;
    int32_t m_minDrawnDepth = -1;
    int32_t m_maxDrawnDepth = -1;
    struct DragDropData
    {
        TreeNode* pParent = nullptr;
        TreeNode* pAfterChildNode = nullptr;
    } m_dragDropData;

    DECLARE_PROPERTY(TreePanel, xpf::ExpandTrigger, ExpandTrigger, ExpandTrigger::DoubleClick, Invalidates::None);
    Tween<float> m_itemToolbarExpandAnimation;
    enum class ItemToolbarState
    {
        Collapsed,
        Expanding,
        Expanded,
    };
    ItemToolbarState m_itemToolbarState = ItemToolbarState::Collapsed;
    TreeNode* m_itemToolbarOnNode = nullptr;
    rectf_t m_deleteItemButtonRect;
    rectf_t m_moveItemButtonRect;

    struct TreePanelInteractiveNode
    {
        float y = 0;
        enum class Action
        {
            Move,
            DeleteAnimation,
            Dragging,
        };
        Action action = Action::Move;
        time_t startTime = -1;
        seconds_t duration = 0;
        bool expandOnceComplete = false;
        float x = 0;
    };

    std::unordered_map<TreeNode*, TreePanelInteractiveNode> m_animatingNodes;
    xpf::Color m_shadowColor = xpf::Colors::Black.with_alpha(64);

public:
    TreePanel() : ScrollPanelMixIn<UIElement>(UIElementType::TreePanel)
    {
        m_ScrollOptions.Set(ScrollOptions::FittedScroll);
        m_acceptsFocus = true;
        m_clippingEnabled = true;
        m_HorizontalScrollbarEnabled = false;
        m_itemToolbarExpandAnimation.From(0);
    }

    void SetData(const std::shared_ptr<TreeData>& sp) { m_spData = sp; InvalidateLayout(); }

#pragma region layout & drawing
public:
    virtual v2_t OnMeasure(v2_t insideSize) override
    {
        if (m_HorizontalAlignment != HorizontalAlignment::Center &&
            m_HorizontalAlignment != HorizontalAlignment::Stretch)
            insideSize.x = m_insideRect.width;

        if (m_VerticalAlignment != VerticalAlignment::Center &&
            m_VerticalAlignment != VerticalAlignment::Stretch)
            insideSize.y = m_insideRect.height;

        m_ContentSize.Set(m_spData->Measure(insideSize.w));
        if (m_ContentSize.Get().y < m_insideRect.height)
            m_VerticalScrollPosition = 0;

        return insideSize;
    }

    virtual void OnArrange(v2_t size) override
    {
        float y = m_insideRect.top() - m_VerticalScrollPosition;
        m_drawnNodes.clear();
        m_minDrawnDepth = m_maxDrawnDepth = -1;
        bool addNextOneForLast = false;

        TreeNode* pPrevNode = nullptr;
        for (TreeNode* pNode : m_spData->GetVisibleNodes())
        {
            pNode->m_y = y;
            float height = pNode->m_height;
            m_minDrawnDepth = std::min(m_minDrawnDepth, pNode->m_depth);
            m_maxDrawnDepth = std::max(m_maxDrawnDepth, pNode->m_depth);
            if (addNextOneForLast)
            {
                m_drawnNodes.push_back(pNode);
                break;
            }
            else if (y + height < m_insideRect.top())
            {
                // noop - skipping
            }
            else if (y > m_insideRect.bottom())
            {
                m_drawnNodes.push_back(pNode);
                addNextOneForLast = true;
            }
            else
            {
                if (m_drawnNodes.empty() && pPrevNode != nullptr)
                    m_drawnNodes.push_back(pPrevNode);

                m_drawnNodes.push_back(pNode);
            }
            pPrevNode = pNode;
            y += height;
        }

        m_ViewPortSize.Set(m_insideRect.size());
        ArrangeScrollbars(m_insideRect);
    }

    virtual void OnUpdateVisuals(IRenderer& renderer) override
    {
        for (const auto& pNode : m_spData->GetVisibleNodes())
            pNode->UpdateVisuals(renderer);

        ScrollPanelMixIn::OnUpdateVisuals(renderer);
    }

    virtual void OnDraw(IRenderer& renderer) override
    {
        v2_t mousePos = GetMousePosition(renderer);

        DrawNodes(renderer, mousePos);
        DrawActiveNodes(renderer, mousePos);
        DrawScrollbars(renderer);
        StateManager(renderer, mousePos);

#if TREEPANEL_DEBUG
        switch (m_mouseState)
        {
            case MouseState::LookingForClickStart: renderer.DrawText("LookingForClickStart", 200, 0, "", 20, xpf::Colors::XpfRed); break;
            case MouseState::LookingForClickEnd: renderer.DrawText("LookingForClickEnd", 200, 0, "", 20, xpf::Colors::XpfRed); break;
            case MouseState::LookingForDoubleClickStart: renderer.DrawText("LookingForDoubleClickStart", 200, 0, "", 20, xpf::Colors::XpfRed); break;
            case MouseState::LookingForDoubleClickEnd: renderer.DrawText("LookingForDoubleClickEnd", 200, 0, "", 20, xpf::Colors::XpfRed); break;
        }

        if (m_pNode_Hover != nullptr)
            renderer.DrawText("Hover:" + m_pNode_Hover->GetText(), 200, 20, "", 20, xpf::Colors::Black);
        if (m_pNode_LeftButtonDown != nullptr)
            renderer.DrawText("Left:" + m_pNode_LeftButtonDown->GetText(), 200, 40, "", 20, xpf::Colors::Black);
#endif
    }

protected:
    void DrawNode(IRenderer& renderer, float y, TreeNode* pNode, v2_t mousePos, bool isMouseInside)
    {
        float x = m_insideRect.x;

        bool isHovering = false;
        xpf::Color background = xpf::Colors::Transparent;

        if (m_animatingNodes.contains(pNode))
            return;

        if (m_selectedNodes.contains(pNode))
        {
            if (m_mouseState == MouseState::LookingForClickEnd)
                background = pNode->m_background_selecting.Get();
            else
                background = pNode->m_background_selected.Get();
        }

        if (isMouseInside && pNode->HitTest(mousePos))
        {
            isHovering = true;
            if (m_pNode_Hover != pNode)
            {
                m_pNode_Hover = pNode;
                m_time_HoverStartTime = Time::GetTime();
            }
            
            if (m_pNode_Dragging == nullptr)
            {
                if (!background.is_transparent())
                    background = xpf::math::midpoint(background, pNode->m_background_hover.Get());
                else
                    background = pNode->m_background_hover.Get();
            }
        }

        if (m_pNode_Dragging != nullptr)
        {
            if (m_dragDropData.pParent == pNode)
                background = xpf::Colors::XpfRed;
            else if (m_dragDropData.pAfterChildNode == pNode)
                background = xpf::Colors::LimeGreen;
        }

        float lineWidth = m_insideRect.width;
        if (m_itemToolbarState != ItemToolbarState::Collapsed && pNode == m_itemToolbarOnNode)
        {
            float dx = 1.0;
            if (m_itemToolbarExpandAnimation.IsPlaying())
            {
                dx = m_itemToolbarExpandAnimation.Read();
            }
            else if (m_itemToolbarState == ItemToolbarState::Expanding)
            {
                m_itemToolbarState = ItemToolbarState::Expanded;
            }

            float buttonSize = pNode->m_height;
            lineWidth -= dx * (buttonSize + buttonSize);

            DrawMoveButton(renderer, x + lineWidth, y, buttonSize, buttonSize, mousePos);
            DrawDeleteButton(renderer, x + lineWidth + buttonSize, y, buttonSize, buttonSize, mousePos);
        }

        float lineHeight = pNode->m_height;
        if (!background.is_transparent())
            renderer.DrawRectangle(x, y, lineWidth, lineHeight, background);

        pNode->DrawForeground(renderer, x, y, isHovering, pNode == m_pSelectedNode);

        if (pNode == m_pSelectedNode && IsInFocus())
        {
            DrawFocusRectangle(renderer, rectf_t{x, y, lineWidth, lineHeight}.shrink(1), CornerRadius(0));
        }
    }

    void DrawNodes(IRenderer& renderer, v2_t mousePos)
    {
        const bool isMouseInside = IsMouseInside();
        float y = m_insideRect.y - m_VerticalScrollPosition;
        const bool needToDrawInsertPoint = m_pNode_Dragging != nullptr;
        m_dragDropData.pParent = nullptr;
        m_dragDropData.pAfterChildNode = nullptr;
        bool dragDropTargetFound = false;
        for (TreeNode* pNode : m_drawnNodes)
        {
            if (m_animatingNodes.contains(pNode))
            {
                if (m_animatingNodes[pNode].action == TreePanelInteractiveNode::Action::Dragging)
                    continue;
                if (m_animatingNodes[pNode].action == TreePanelInteractiveNode::Action::Move ||
                    m_animatingNodes[pNode].action == TreePanelInteractiveNode::Action::DeleteAnimation)
                {
                    y += pNode->m_height;
                    continue;
                }
            }

            pNode->m_y = y;
            if (needToDrawInsertPoint && pNode->HitTest(mousePos))
            {
                m_dragDropData.pParent = pNode->m_pParent;
                m_dragDropData.pAfterChildNode = pNode;
                y += m_pNode_Dragging->m_height;
                dragDropTargetFound = true;
            }

            DrawNode(renderer, y, pNode, mousePos, isMouseInside);
            y += pNode->m_height;
        }

        if (needToDrawInsertPoint && !dragDropTargetFound)
        {
            if (mousePos.y > m_insideRect.top())
            {
                for (TreeNode* pNode : m_spData->GetVisibleNodes())
                {
                    if (pNode->GetType() == TreeNodeType::Parent)
                    {
                        m_dragDropData.pParent = pNode;
                    }
                    else
                    {
                        if (m_dragDropData.pParent == pNode->m_pParent)
                            m_dragDropData.pAfterChildNode = pNode;
                        else
                            m_dragDropData.pParent = nullptr;
                    }
                }
            }

        }
    }

    void DrawActiveNodes(IRenderer& renderer, v2_t mousePos)
    {
        bool isMouseInside = IsMouseInside();

        for (auto iter = m_animatingNodes.begin(); iter != m_animatingNodes.end();)
        {
            float shadowSize = 4;
            TreeNode* pNode = iter->first;
            TreePanelInteractiveNode& data = iter->second;
            float yy = pNode->m_y;
            float xx = m_insideRect.x;
            xpf::Color background = pNode->m_background_selected.Get().with_alpha(220);
            xpf::Color shadowColor = m_shadowColor;
            float lineWidth = m_insideRect.width;
            bool removeFromAnimation = false;
            bool deleteNode = false;

            switch (data.action)
            {
                case TreePanelInteractiveNode::Action::Dragging:
                {
                    //if (isMouseInside)
                        data.y = mousePos.y - pNode->m_height * .5f;

                    int32_t depthChange = int32_t(round((mousePos.x - data.x) / TreeParentNode::m_indentWidth));
                    if (depthChange != 0)
                    {
                        int32_t depth = (pNode->m_depth + depthChange);
                        int32_t mindepth = 0;
                        if (m_dragDropData.pParent != nullptr)
                            mindepth = m_dragDropData.pParent->m_depth;
                        depth = xpf::math::clamp(depth, mindepth, m_maxDrawnDepth);

                        xx += depth * TreeParentNode::m_indentWidth;
                    }

                    yy = data.y;
                    break;
                }
                case TreePanelInteractiveNode::Action::Move:
                {
                    // renderer.DrawLine({
                    //     {{xx + m_insideRect.width, pNode->m_y}, xpf::Colors::White.with_alpha(120),2},
                    //     {{xx + m_insideRect.width, pNode->m_y + pNode->m_height}, xpf::Colors::White.with_alpha(120),2},
                    //     {{xx + 1, pNode->m_y + pNode->m_height}, xpf::Colors::White.with_alpha(120),2},
                    //     {{xx + 1, pNode->m_y + pNode->m_height}, xpf::Colors::Black.with_alpha(120),2},
                    //     {{xx + 1, pNode->m_y}, xpf::Colors::Black.with_alpha(120),2},
                    //     {{xx + m_insideRect.width, pNode->m_y}, xpf::Colors::Black.with_alpha(120),2},
                    // });
                    renderer.DrawRectangle(xx, pNode->m_y, m_insideRect.width, pNode->m_height, xpf::Colors::White.with_alpha(120));

                    if (data.startTime < 0)
                    {
                        data.startTime = Time::GetTime();
                        data.duration = xpf::math::clamp<float>(std::abs(pNode->m_y - data.y) / 100, 0, 0.75);
                    }

                    seconds_t deltaTime = Time::GetTime() - data.startTime;

                    if (deltaTime < (data.duration * .5))
                    {
                        yy = xpf::math::map<float>(deltaTime, 0, data.duration * .5, data.y, pNode->m_y);
                    }
                    else if (deltaTime < (data.duration))
                    {
                        yy = pNode->m_y;
                        if (deltaTime > (data.duration * .75))
                        {
                            float t = (deltaTime - (data.duration * .5)) / (data.duration * .25);
                            shadowSize = xpf::math::lerp<float>(shadowSize, 0.0, t);
                        }
                    }
                    else
                    {
                        pNode->SetIsExpanded(data.expandOnceComplete);
                        NodeSelected(pNode);
                        removeFromAnimation = true;
                    }
                    break;
                }
                case TreePanelInteractiveNode::Action::DeleteAnimation:
                {
                    if (data.startTime < 0)
                    {
                        data.startTime = Time::GetTime();
                        data.duration = .25f;
                        data.y = data.startTime + data.duration;
                    }

                    lineWidth = xpf::math::lerp<float>(m_insideRect.width, 0, xpf::math::cubic_ease_out((Time::GetTime() - data.startTime) / data.duration));
                    renderer.DrawRectangle(xx, yy, lineWidth, pNode->m_height, xpf::Colors::Red);

                    if (data.y < Time::GetTime())
                    {
                        removeFromAnimation = true;
                        m_spData->DeleteNode(pNode);
                        pNode = nullptr;
                    }
                    break;
                }
            }

            if (pNode != nullptr)
            {
                if (data.action != TreePanelInteractiveNode::Action::DeleteAnimation)
                {
                    rectf_t rect = rectf_t::from_points(m_insideRect.left(), yy, m_insideRect.right() - shadowSize, yy + pNode->m_height);
                    if (shadowSize > 0)
                        renderer.DrawRectangle(rect.move(shadowSize, shadowSize), shadowColor);

                    renderer.DrawRectangle(rect, background);

                    pNode->DrawForeground(renderer, xx, yy, /*hover:*/ false, /*isSelected:*/ false);

                    if (data.action == TreePanelInteractiveNode::Action::Dragging)
                    {
                        DrawMoveButton(renderer, m_insideRect.right() - pNode->m_height - pNode->m_height, yy, pNode->m_height, pNode->m_height, v2_t(0,0));
                    }
                }
            }

            if (removeFromAnimation)
            {
                iter = m_animatingNodes.erase(iter);
                InvalidateLayout();
            }
            else
            {
                iter++;
            }
        }
    }

    void DrawMoveButton(IRenderer& renderer, float x, float y, float width, float height, v2_t mousePos)
    {
        TextDescription desc;
        desc.foreground = xpf::Colors::White;
        desc.fontName = "segoeicons";
        desc.fontSize = 14;
        desc.trimming = TextTrimming::None;
        desc.renderOptions = FontRenderOptions::SizeInPixels;

        // delete button
        m_moveItemButtonRect = rectf_t{x, y, width, height};
        xpf::Color moveButtonBackground = xpf::Colors::DodgerBlue;
        if (m_moveItemButtonRect.is_inside(mousePos))
            moveButtonBackground = moveButtonBackground.mix(xpf::Colors::White, 0.8);

        renderer.DrawRectangle(x, y, width, height, moveButtonBackground);
        renderer.DrawText(
            "\ue700",
            round(x + (width - desc.fontSize) * .5)+.5f,
            round(y + (height - desc.fontSize) * .5),
            desc);
    }

    void DrawDeleteButton(IRenderer& renderer, float x, float y, float width, float height, v2_t mousePos)
    {
        TextDescription desc;
        desc.foreground = xpf::Colors::White;
        desc.fontName = "segoeicons";
        desc.fontSize = 14;
        desc.trimming = TextTrimming::None;
        desc.renderOptions = FontRenderOptions::SizeInPixels;

        // delete button
        m_deleteItemButtonRect = rectf_t{x, y, width, height};
        xpf::Color deleteButtonBackground = xpf::Colors::Red;
        if (m_deleteItemButtonRect.is_inside(mousePos))
            deleteButtonBackground = xpf::Colors::Tomato;

        renderer.DrawRectangle(x, y, width, height, deleteButtonBackground);
        renderer.DrawText(
            "\ue107",
            round(x + (width - desc.fontSize) * .5)+.5f,
            round(y + (height - desc.fontSize) * .5),
            desc);
    }

    virtual void DrawFocus(IRenderer&) override { }
#pragma endregion

protected:
    std::unordered_set<TreeNode*> m_selectedNodes;
    TreeNode* m_pSelectedNode = nullptr;
    TreeNode* m_pSelectionStart = nullptr;
    TreeNode* m_pNode_Hover = nullptr;
    TreeNode* m_pNode_LeftButtonDown = nullptr;
    time_t m_time_LeftButtonDown = 0;
    time_t m_time_LeftButtonUp = 0;
    time_t m_time_HoverStartTime = 0;
    float m_menuExpanding = 0;
    v2_t m_previousMousePosition;
    TreeNode* m_pNode_Dragging = nullptr;
    TreeNode* m_pNode_DeleteClicked = nullptr;

    enum class MouseState
    {
        LookingForClickStart,
        LookingForClickEnd,
        LookingForDoubleClickStart,
        LookingForDoubleClickEnd,
    };

    MouseState m_mouseState = MouseState::LookingForClickStart;

    void NodeSelected(TreeNode* pNode)
    {
        m_selectedNodes.clear();
        if (pNode != nullptr)
            m_selectedNodes.insert(pNode);
        m_pSelectedNode = pNode;
        m_pSelectionStart = pNode;
        InvalidateLayout();
    }

    void NodeToggleExpansion(TreeNode* pNode)
    {
        if (pNode->GetType() == TreeNodeType::Parent)
        {
            static_cast<TreeParentNode*>(pNode)->ToggleIsExpanded();
            InvalidateLayout();
        }
        else
        {
            NodeSelected(pNode);
        }
    }

    virtual void OnFocusGain() override
    {
        if (IsInFocus())
            return;

        if (m_pSelectedNode == nullptr)
        {
            for (TreeNode* pNode : m_spData->GetVisibleNodes())
            {
                NodeSelected(pNode);
                return;
            }
        }
    }

    time_t m_time_KeyDown = 0;
    time_t m_waitBeforeRepat = .25;
    KeyCode m_keydown_Code = KeyCode::None;

    bool IsKeyPressed(KeyCode keyCode)
    {
        if (!InputService::IsKeyDown(keyCode))
        {
            if (m_keydown_Code == keyCode)
            {
                m_waitBeforeRepat = .25;
                m_time_KeyDown = 0;
            }
            return false;
        }

        if (m_keydown_Code != keyCode || (Time::GetTime() - m_time_KeyDown) > m_waitBeforeRepat)
        {
            m_time_KeyDown = Time::GetTime();
            m_keydown_Code = keyCode;
            m_waitBeforeRepat *= .9;
            m_waitBeforeRepat = std::min(m_waitBeforeRepat, 0.1);
            return true;
        }
        return false;
    }

    TreeNode* PrevNode(TreeNode* pStart)
    {
        TreeNode* pPrevNode = nullptr;
        for (TreeNode* pNode : m_spData->GetVisibleNodes())
        {
            if (pNode == pStart && pPrevNode != nullptr)
                return pPrevNode;

            pPrevNode = pNode;
        }

        return pStart;
    }

    TreeNode* NextNode(TreeNode* pStart)
    {
        bool selectNextOne = false;
        for (TreeNode* pNode : m_spData->GetVisibleNodes())
        {
            if (selectNextOne)
                return pNode;
            else if (pNode == pStart)
                selectNextOne = true;
        }

        return pStart;
    }

    void NodesBetween(TreeNode* pStart, TreeNode* pEnd, std::function<void(TreeNode*)>&& fn)
    {
        if (pStart->m_y > pEnd->m_y)
            std::swap(pStart, pEnd);

        bool startFound = false;
        for (TreeNode* pNode : m_drawnNodes)
        {
            if (startFound)
            {
                fn(pNode);
                if (pNode == pEnd)
                    return;
            }
            else if (pNode == pStart)
            {
                startFound = true;
                fn(pNode);
                if (pNode == pEnd)
                    return;
            }
        }
    }

    void VerticalScrollToNode(const TreeNode* pNode)
    {
        if (pNode->m_y < 0)
            VerticalScrollTo(pNode->m_y + m_VerticalScrollPosition.Get());
        else if (pNode->m_y + pNode->m_height > m_ViewPortSize.Get().h)
            VerticalScrollTo(pNode->m_y + pNode->m_height - m_ViewPortSize.Get().h + m_VerticalScrollPosition.Get());
    }

    void StateManager(IRenderer& renderer, v2_t mousePos)
    {
        if (!GetIsHitTestVisible())
            return;

        const bool mouseMovedSignificantly = ((m_previousMousePosition - mousePos).magnitude_squared() > 1.0);
        m_previousMousePosition = mousePos;
        // keyboard
        if (IsInFocus())
        {
            bool inSelectionMode = (InputService::IsKeyDown(KeyCode::LeftShift) || InputService::IsKeyDown(KeyCode::RightShift));
            const bool keyUp = s_up.IsKeyPressed();
            const bool keyDown = s_down.IsKeyPressed();
            if (keyUp || keyDown)
            {
                TreeNode* pNewSelectedNode = keyUp ? PrevNode(m_pSelectedNode) : NextNode(m_pSelectedNode);
                if (pNewSelectedNode == m_pSelectedNode)
                {
                    // no change - noop
                }
                else if (inSelectionMode)
                {
                    m_selectedNodes.clear();
                    NodesBetween(pNewSelectedNode, m_pSelectionStart, [&](TreeNode* pNode) {
                        m_selectedNodes.insert(pNode);
                    });
                    m_pSelectedNode = pNewSelectedNode;
                }
                else
                {
                    NodeSelected(pNewSelectedNode);
                }

                VerticalScrollToNode(pNewSelectedNode);
            }
            else if (s_right.IsKeyPressed())
            {
                for (TreeNode* pNode : m_selectedNodes)
                    pNode->SetIsExpanded(true);

                InvalidateLayout();
            }
            else if (s_left.IsKeyPressed())
            {
                if (m_pSelectedNode != nullptr && m_pSelectedNode->m_pParent != nullptr)
                {
                    if (m_pSelectedNode->GetType() == TreeNodeType::Leaf)
                    {
                        NodeSelected(m_pSelectedNode->m_pParent);
                        return;
                    }
                    else if (m_pSelectedNode->GetType() == TreeNodeType::Parent && !m_pSelectedNode->GetIsExpanded())
                    {
                        NodeSelected(m_pSelectedNode->m_pParent);
                        return;
                    }
                }

                for (TreeNode* pNode : m_selectedNodes)
                    pNode->SetIsExpanded(false);
                InvalidateLayout();
            }
            else if (s_space.IsKeyPressed())
            {
                for (TreeNode* pNode : m_selectedNodes)
                    pNode->ToggleIsExpanded();
                InvalidateLayout();
            }
        }

        const bool leftIsDown = InputService::IsMouseButtonDown(xpf::MouseButton::Left);
        const bool isMouseInside = IsMouseInside();
        bool inAddToSelectionMode =
        #if defined(PLATFORM_APPLE)
            InputService::IsKeyDown(KeyCode::LeftControl) || InputService::IsKeyDown(KeyCode::RightControl) ||
            InputService::IsKeyDown(KeyCode::LeftSuper) || InputService::IsKeyDown(KeyCode::RightSuper);
        #else
            InputService::IsKeyDown(KeyCode::LeftControl) || InputService::IsKeyDown(KeyCode::RightControl);
        #endif

        switch (m_mouseState)
        {
            case MouseState::LookingForClickStart:
                if (leftIsDown && isMouseInside)
                {
                    if (m_itemToolbarOnNode == m_pNode_Hover && m_itemToolbarOnNode != nullptr)
                    {
                        if (m_deleteItemButtonRect.is_inside(mousePos))
                        {
                            m_pNode_DeleteClicked = m_itemToolbarOnNode;
                        }
                        else if (m_moveItemButtonRect.is_inside(mousePos))
                        {
                            m_pNode_Dragging = m_itemToolbarOnNode;

                            TreePanelInteractiveNode action;
                            action.action = TreePanelInteractiveNode::Action::Dragging;
                            action.x = mousePos.x;
                            action.y = mousePos.y;
                            action.expandOnceComplete = m_pNode_Dragging->GetIsExpanded();
                            m_animatingNodes[m_pNode_Dragging] = std::move(action);

                            m_pNode_Dragging->SetIsExpanded(false);
                            InvalidateLayout();
                            m_mouseState = MouseState::LookingForClickEnd;
                        }

                        m_itemToolbarOnNode = nullptr;
                        m_pSelectedNode = nullptr;
                        m_pNode_LeftButtonDown = nullptr;
                        m_pNode_Hover = nullptr;
                        m_selectedNodes.clear();
                    }
                    else if (m_itemToolbarState == ItemToolbarState::Expanded || m_itemToolbarState == ItemToolbarState::Expanding)
                    {
                        m_itemToolbarOnNode = nullptr;
                        m_itemToolbarState = ItemToolbarState::Collapsed;
                    }
                }

                if (leftIsDown && m_pNode_Hover != nullptr && isMouseInside)
                {
                    Focus();
                    m_selectedNodes.clear();
                    m_pNode_LeftButtonDown = m_pNode_Hover;
                    m_time_LeftButtonDown = Time::GetTime();
                    m_mouseState = MouseState::LookingForClickEnd;
                }
                else
                {
                    m_pNode_LeftButtonDown =nullptr;
                }
                break;
            case MouseState::LookingForClickEnd:
            {
                if (m_itemToolbarOnNode != m_pNode_Hover && m_itemToolbarOnNode != nullptr)
                {
                    m_itemToolbarOnNode = nullptr;
                    m_itemToolbarState = ItemToolbarState::Collapsed;
                }

                // scroll when dragging mouse up or down during selection
                if (!IsMouseInside() &&
                    m_insideRect.left() <= mousePos.x && mousePos.x < m_insideRect.right() &&
                    !m_drawnNodes.empty() &&
                    m_ContentSize.Get().y > m_insideRect.h)
                {
                    float newPosition = 0;
                    if (mousePos.y < m_insideRect.top()) // above window
                    {
                        m_pNode_Hover = m_drawnNodes.front();
                        newPosition = m_VerticalScrollPosition - 3;
                        VerticalScrollTo(newPosition);
                    }
                    else if (mousePos.y > m_insideRect.bottom()) // below window
                    {
                        m_pNode_Hover = m_drawnNodes.back();
                        newPosition = m_VerticalScrollPosition + 3;
                        newPosition = xpf::math::clamp<float>(newPosition, 0.0, m_ContentSize.Get().y - m_insideRect.h);
                        VerticalScrollTo(newPosition);
                    }
                }

                if (m_pNode_Dragging != nullptr)
                {
                    if (!leftIsDown)
                    {
                        m_animatingNodes[m_pNode_Dragging].y = mousePos.y;
                        m_animatingNodes[m_pNode_Dragging].action = TreePanelInteractiveNode::Action::Move;
                        NodeSelected(m_pNode_Dragging);
                        m_pNode_LeftButtonDown = nullptr;
                        m_pNode_Dragging = nullptr;
                    }
                    return;
                }

                if (m_pNode_LeftButtonDown != nullptr)
                {
                    if (m_pNode_Hover != m_pNode_LeftButtonDown)
                    {
                        m_selectedNodes.clear();
                        bool isPainting = false;
                        for (TreeNode* pNode : m_drawnNodes)
                        {
                            bool wasPainting = false;
                            if (m_pNode_Hover != nullptr && m_pNode_LeftButtonDown != nullptr)
                            {
                                wasPainting = isPainting;
                                if (pNode == m_pNode_Hover || pNode == m_pNode_LeftButtonDown)
                                    isPainting = !isPainting;

                                if (isPainting || wasPainting)
                                    m_selectedNodes.insert(pNode);
                            }
                        }
                    }
                    else
                    {
                        NodeSelected(m_pNode_Hover);
                    }
                }

                if (leftIsDown)
                {
                    if (!mouseMovedSignificantly &&
                        m_itemToolbarState == ItemToolbarState::Collapsed &&
                        m_pNode_Hover == m_pNode_LeftButtonDown &&
                        (Time::GetTime() - m_time_LeftButtonDown > 0.5f))
                    {
                       m_itemToolbarState = ItemToolbarState::Expanding;
                       m_itemToolbarOnNode = m_pNode_Hover;
                       m_itemToolbarExpandAnimation.Clear().To(1.0f, 0.25f, xpf::math::cubic_ease_inOut).Play();
                    }

                    return;
                }

                if (m_pNode_DeleteClicked != nullptr)
                {
                    if (m_deleteItemButtonRect.is_inside(mousePos))
                    {
                        m_animatingNodes[m_pNode_DeleteClicked].action = TreePanelInteractiveNode::Action::DeleteAnimation;
                        NodeSelected(nullptr);
                    }
                    m_pNode_DeleteClicked = nullptr;
                    return;
                }

                if (m_pNode_Hover != m_pNode_LeftButtonDown)
                {
                    m_pSelectedNode = m_pNode_Hover;
                    m_mouseState = MouseState::LookingForClickStart;
                    return; // NodeSelected(nullptr);
                }

                if (m_ExpandTrigger == ExpandTrigger::SingleClick)
                {
                    NodeSelected(m_pNode_Hover);
                    NodeToggleExpansion(m_pNode_Hover);
                    m_pNode_LeftButtonDown = nullptr;
                    m_mouseState = MouseState::LookingForClickStart;
                }
                else
                {
                    NodeSelected(m_pNode_Hover);

                    if (Time::GetTime() - m_time_LeftButtonDown < .25)
                    {
                        m_mouseState = MouseState::LookingForDoubleClickStart;
                        m_time_LeftButtonUp = Time::GetTime();
                    }
                    else
                    {
                        m_pNode_LeftButtonDown = nullptr;
                        m_mouseState = MouseState::LookingForClickStart;
                    }
                }
                break;
            }
            case MouseState::LookingForDoubleClickStart:
                if (leftIsDown)
                {
                    if (m_pNode_Hover != nullptr && m_pNode_Hover == m_pNode_LeftButtonDown)
                    {
                        if (Time::GetTime() - m_time_LeftButtonUp < .25)
                        {
                            m_mouseState = MouseState::LookingForDoubleClickEnd;
                            m_time_LeftButtonDown = Time::GetTime();
                        }
                        else
                        {
                            m_pNode_LeftButtonDown = nullptr;
                            m_mouseState = MouseState::LookingForClickStart;
                        }
                    }
                }
                else if (Time::GetTime() - m_time_LeftButtonUp > .25)
                {
                    m_pNode_LeftButtonDown = nullptr;
                    m_mouseState = MouseState::LookingForClickStart;
                }
                break;
            case MouseState::LookingForDoubleClickEnd:
                if (!leftIsDown && m_pNode_Hover != nullptr && m_pNode_Hover == m_pNode_LeftButtonDown)
                {
                    if (Time::GetTime() - m_time_LeftButtonDown < .25)
                    {
                        NodeToggleExpansion(m_pNode_Hover);
                    }

                    m_pNode_Hover = nullptr;
                    m_pNode_LeftButtonDown = nullptr;
                    m_mouseState = MouseState::LookingForClickStart;
                    m_time_LeftButtonDown = 0;
                }
                break;
        }
    }
};

} // xpf