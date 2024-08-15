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
    if (m_settings.arrowExpand)
    {
        flags |= ImGuiTreeNodeFlags_OpenOnArrow;
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
    // The order here is unusual due to imgui weirdness; see the second-to-last paragraph in https://kahwei.dev/2022/06/20/imgui-tree-node/
    bool expanded = ImGui::TreeNodeEx(node, flags, "%s", name.str().c_str());
    if (ImGui::IsItemClicked()) { 
        DevTools::get()->selectNode(node);
        selected = true;
    }
    if (ImGui::IsItemHovered() && (m_settings.alwaysHighlight || ImGui::IsKeyDown(ImGuiMod_Shift))) {
        DevTools::get()->highlightNode(node, HighlightMode::Hovered);
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

    this->drawTreeBranch(CCDirector::get()->getRunningScene(), 0);
}
