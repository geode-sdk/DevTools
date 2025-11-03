#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include "../platform/utils.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <Geode/utils/string.hpp>

#ifndef GEODE_IS_WINDOWS
#include <cxxabi.h>
#endif

using namespace geode::prelude;

void DevTools::drawTreeBranch(CCNode* node, size_t index, bool drag) {
    if (!searchBranch(node)) {
        return;
    }

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

    if (auto dragged = DevTools::get()->getDraggedNode(); dragged && dragged != node && !drag) {
        float mouse = ImGui::GetMousePos().y;
        float cursor = ImGui::GetCursorPosY() + ImGui::GetWindowPos().y - ImGui::GetScrollY();

        if (mouse <= cursor + 18 && mouse > cursor) {
            flags |= ImGuiTreeNodeFlags_Selected;

            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                for(CCNode* n = node;; n = n->getParent()) {
                    if (n == dragged) { // can't drag a parent into its own child
                        break;
                    } else if (n == nullptr) {
                        auto parent = dragged->getParent();
                        dragged->removeFromParentAndCleanup(false);
                        parent->updateLayout();
                        node->addChild(dragged);
                        node->updateLayout();
                        break;
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
    float height = ImGui::GetItemRectSize().y;


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

    if (m_settings.enableMoving) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.75f));
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);

        std::string btnText = (U8STR(FEATHER_MENU"##menu_")) + std::to_string((long)node);
        ImGui::Button(btnText.c_str(), {0, height});

        isDrag = ImGui::IsItemActive();
        if (isDrag) {
            DevTools::get()->setDraggedNode(node);
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
    }

    if (expanded) {
        if (m_settings.attributesInTree) {
            this->drawNodeAttributes(node);
        }
        size_t i = 0;
        for (auto& child : CCArrayExt<CCNode*>(node->getChildren())) {
            this->drawTreeBranch(child, i++, drag || isDrag);
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

    this->drawTreeBranch(CCDirector::get()->getRunningScene(), 0, false);

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        DevTools::get()->setDraggedNode(nullptr);
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