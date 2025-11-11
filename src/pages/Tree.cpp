#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include "../platform/utils.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <Geode/utils/string.hpp>
#include "../ImGui.hpp"

#ifndef GEODE_IS_WINDOWS
#include <cxxabi.h>
#endif

using namespace geode::prelude;

std::string formatNodeName(CCNode* node, size_t index) {
    std::string name = fmt::format("[{}] {} ", index, geode::cocos::getObjectName(node));
    if (node->getTag() != -1) {
        name += fmt::format("({}) ", node->getTag());
    }
    if (node->getID().size()) {
        name += fmt::format("\"{}\" ", node->getID());
    }
    if (node->getChildrenCount()) {
        name += fmt::format("<{}> ", node->getChildrenCount());
    }
    return name;
}

bool isNodeParentOf(CCNode* parent, CCNode* child) {
    for (CCNode* cur = child; cur != nullptr; cur = cur->getParent()) {
        if (cur == parent) {
            return true;
        }
    }
    return false;
}

void DevTools::drawTreeBranch(CCNode* node, size_t index, bool drag, bool visible) {
    if (!this->searchBranch(node)) {
        return;
    }

    visible = node->isVisible() and visible;

    auto selected = DevTools::get()->getSelectedNode() == node;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
    if (selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!node->getChildrenCount()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (m_settings.arrowExpand) {
        flags |= ImGuiTreeNodeFlags_OpenOnArrow;
    }

    bool drawSeparator = false;

    if (auto dragged = this->getDraggedNode(); dragged && dragged != node && !drag) {
        float mouseY = ImGui::GetMousePos().y;
        float cursorY = ImGui::GetCursorPosY() + ImGui::GetWindowPos().y - ImGui::GetScrollY();
        float height = ImGui::GetTextLineHeight();
        float spacing = ImGui::GetStyle().FramePadding.y;

        if (mouseY > cursorY && mouseY <= cursorY + height + spacing) {
            if (mouseY <= cursorY + height) {
                flags |= ImGuiTreeNodeFlags_Selected;

                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !isNodeParentOf(dragged, node)) {
                    auto parent = dragged->getParent();
                    dragged->removeFromParentAndCleanup(false);
                    node->addChild(dragged);
                    parent->updateLayout();
                    node->updateLayout();
                }
            } else {
                drawSeparator = true;

                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !isNodeParentOf(dragged, node)) {
                    // place the dragged node right after `node`
                    auto newParent = node->getParent();
                    if (newParent) {
                        auto oldParent = dragged->getParent();
                        auto children = newParent->getChildrenExt();
                        for (int i = index + 1; i < children.size(); i++) {
                            children[i]->m_uOrderOfArrival++;
                        }
                        dragged->removeFromParentAndCleanup(false);
                        newParent->addChild(dragged, node->getZOrder());
                        dragged->m_uOrderOfArrival = node->m_uOrderOfArrival + 1;

                        if (oldParent != newParent) oldParent->updateLayout();
                        newParent->updateLayout();
                    }
                }
            }
        }
    }

    if (m_searchQuery.empty()) {
        ImGui::SetNextItemOpen(m_nodeOpen.contains(node) && m_nodeOpen[node]);
    }
    else if (m_searchQuery != m_prevQuery) {
        ImGui::SetNextItemOpen(true);
    }

    auto alpha = ImGui::GetStyle().DisabledAlpha;
    ImGui::GetStyle().DisabledAlpha = node->isVisible() ? alpha + 0.15f : alpha;

    ImGui::BeginDisabled(!visible);
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false); // Bypass iteract blocking in imgui

    const auto name = formatNodeName(node, index);
    // The order here is unusual due to imgui weirdness; see the second-to-last paragraph in https://kahwei.dev/2022/06/20/imgui-tree-node/
    bool expanded = ImGui::TreeNodeEx(node, flags, "%s", name.c_str());
    float height = ImGui::GetItemRectSize().y;


    ImGui::GetStyle().DisabledAlpha = alpha;
    ImGui::PopItemFlag(); //ImGuiItemFlags_Disabled
    ImGui::EndDisabled();

    if (ImGui::IsItemClicked()) {
        DevTools::get()->selectNode(node);
        selected = true;

        if (!m_searchQuery.empty()) {
            CCNode* parent = node->getParent();
            while (parent) {
                m_nodeOpen[parent] = true;
                parent = parent->getParent();
            }
        }
    }
    if (ImGui::IsItemToggledOpen() && (m_searchQuery.empty() || m_searchQuery == m_prevQuery)) {
        if (!m_searchQuery.empty() && expanded) {
            CCNode* parent = node->getParent();
            while (parent) {
                m_nodeOpen[parent] = true;
                parent = parent->getParent();
            }
        }
        m_nodeOpen[node] = expanded;
    }

    if (ImGui::IsItemHovered() && (m_settings.alwaysHighlight || ImGui::IsKeyDown(ImGuiMod_Shift))) {
        DevTools::get()->highlightNode(node, HighlightMode::Hovered);
    }

    bool isDrag = false;

    if (m_settings.treeDragReorder && ImGui::IsItemActive()) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, height / 2.f)) {
            isDrag = true;
            if (this->getDraggedNode() != node) {
                this->setDraggedNode(node);
            }
        }
    }

    if (expanded) {
        if (m_settings.attributesInTree) {
            this->drawNodeAttributes(node);
        }
        size_t i = 0;
        for (auto& child : CCArrayExt<CCNode*>(node->getChildren())) {
            this->drawTreeBranch(child, i++, drag || isDrag, visible);
        }
        ImGui::TreePop();
    }
    // on leaf nodes expanded is true
    if (drawSeparator && (!expanded || !node->getChildrenCount())) {
        ImGui::Separator();
    }
}

void DevTools::drawTree() {
#ifdef GEODE_IS_MOBILE
    ImGui::Dummy({0.f, 60.f});
#endif
    m_prevQuery = m_searchQuery;

    auto space = ImGui::GetContentRegionAvail();
    auto height = ImGui::GetFrameHeight();
    ImGui::SetNextItemWidth(space.x - height);
    ImGui::InputTextWithHint("##search", U8STR(FEATHER_SEARCH " Search for a node..."), &m_searchQuery, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine(0, 0);
    if (ImGui::Button(U8STR(FEATHER_X), ImVec2(height, height))) {
        m_searchQuery.clear();
    }

    this->drawTreeBranch(CCDirector::get()->getRunningScene(), 0, false, true);

    if (auto* dragged = this->getDraggedNode()) {
        const auto name = formatNodeName(dragged, 0);
        auto bgColor = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Header]);
        auto textColor = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);

        auto textSize = ImGui::CalcTextSize(name.c_str());
        auto pos = ImGui::GetMousePos() - textSize / 2.f;

        auto* drawing = ImGui::GetWindowDrawList();
        drawing->AddRectFilled(pos, pos + textSize, bgColor);
        drawing->AddText(pos, textColor, name.c_str(), name.c_str() + name.size());
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        this->setDraggedNode(nullptr);
    }
}

bool DevTools::searchBranch(CCNode* node) {
    if (m_searchQuery.empty()) return true;

    std::string name(geode::cocos::getObjectName(node));
    std::string id = node->getID();
    std::string query = m_searchQuery;

    utils::string::toLowerIP(name);
    utils::string::toLowerIP(id);
    utils::string::toLowerIP(query);

    if (name.find(query) != std::string::npos || id.find(query) != std::string::npos) {
        return true;
    }
    for (auto child : node->getChildrenExt<CCNode>()) {
        if (searchBranch(child)) {
            return true;
        }
    }
    return false;
}