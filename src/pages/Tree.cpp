#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include "../platform/utils.hpp"
#include <misc/cpp/imgui_stdlib.h>
#ifndef GEODE_IS_WINDOWS
#include <cxxabi.h>
#endif

using namespace geode::prelude;

void DevTools::drawTreeBranch(CCNode* node, size_t index) {
    if (!searchBranch(node)) {
        return;
    }

    auto selected = DevTools::get()->getSelectedNode() == node;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
    if (selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!node->getChildrenCount())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (m_settings.arrowExpand)
    {
        flags |= ImGuiTreeNodeFlags_OpenOnArrow;
    }

    if (m_searchQuery.empty()) {
        ImGui::SetNextItemOpen(m_nodeOpen.contains(node) && m_nodeOpen[node]);
    }
    else if (m_searchQuery != m_prevQuery) {
        ImGui::SetNextItemOpen(true);
    }

    std::stringstream name;
    name << "[" << index << "] " << geode::cocos::getObjectName(node) << " ";
    if (node->getTag() != -1) {
        name << "(" << node->getTag() << ") ";
    }
    if (node->getID().size()) {
        name << "\"" << node->getID() << "\" ";
    }
    if (node->getChildrenCount()) {
        name << "<" << node->getChildrenCount() << "> ";
    }
    // The order here is unusual due to imgui weirdness; see the second-to-last paragraph in https://kahwei.dev/2022/06/20/imgui-tree-node/
    bool expanded = ImGui::TreeNodeEx(node, flags, "%s", name.str().c_str());
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
    if (ImGui::IsItemHovered() && (m_settings.alwaysHighlight || ImGui::IsKeyDown(ImGuiMod_Shift))) {
        DevTools::get()->highlightNode(node, HighlightMode::Hovered);
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
    if (expanded) {
        if (m_settings.attributesInTree) {
            this->drawNodeAttributes(node);
        }
        size_t i = 0;
        for (auto& child : CCArrayExt<CCNode*>(node->getChildren())) {
            this->drawTreeBranch(child, i++);
        }
        ImGui::TreePop();
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

    this->drawTreeBranch(CCDirector::get()->getRunningScene(), 0);
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