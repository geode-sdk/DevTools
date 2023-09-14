#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#ifndef GEODE_IS_WINDOWS
#include <cxxabi.h>
#endif

using namespace geode::prelude;

std::string getNodeName(CCObject* node) {
#ifdef GEODE_IS_WINDOWS
    return typeid(*node).name() + 6;
#else 
    {
        std::string ret;

        int status = 0;
        auto demangle = abi::__cxa_demangle(typeid(*node).name(), 0, 0, &status);
        if (status == 0) {
            ret = demangle;
        }
        free(demangle);

        return ret;
    }
#endif
}

void DevTools::drawTreeBranch(CCNode* node, size_t index) {
    auto selected = DevTools::get()->getSelectedNode() == node;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
    if (selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!node->getChildrenCount())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    std::stringstream name;
    name << "[" << index << "] " << getNodeName(node) << " ";
    if (node->getTag() != -1) {
        name << "(" << node->getTag() << ") ";
    }
    if (node->getID().size()) {
        name << "\"" << node->getID() << "\" ";
    }
    if (node->getChildrenCount()) {
        name << "<" << node->getChildrenCount() << "> ";
    }
    if (ImGui::TreeNodeEx(
        node, flags, "%s", name.str().c_str()
    )) {
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            if (selected) {
                DevTools::get()->selectNode(nullptr);
                selected = false;
            } else {
                DevTools::get()->selectNode(node);
                selected = true;
            }
        }
        if (ImGui::IsItemHovered() && (
            m_alwaysHighlight || ImGui::IsKeyDown(ImGuiKey_ModShift)
        )) {
            DevTools::get()->highlightNode(node, HighlightMode::Hovered);
        }
        if (m_attributesInTree) {
            this->drawNodeAttributes(node);
        }
        size_t i = 0;
        for (auto& child : CCArrayExt<CCNode*>(node->getChildren())) {
            this->drawTreeBranch(child, i++);
        }
        ImGui::TreePop();
    }
    else if (ImGui::IsItemHovered() && (
        m_alwaysHighlight || ImGui::IsKeyDown(ImGuiKey_ModShift)
    )) {
        DevTools::get()->highlightNode(node, HighlightMode::Hovered);
    }
}

void DevTools::drawTree() {
    this->drawTreeBranch(CCDirector::get()->getRunningScene(), 0);
}
