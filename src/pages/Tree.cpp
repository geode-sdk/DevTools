#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"

USE_GEODE_NAMESPACE();

const char* getNodeName(CCObject* node) {
    return typeid(*node).name() + 6;
}

void DevTools::drawTreeBranch(CCNode* node, size_t index) {
    auto selected = DevTools::get()->getSelectedNode() == node;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
    if (selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
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
        name << "{" << node->getChildrenCount() << "} ";
    }
    if (ImGui::TreeNodeEx(
        node, flags, name.str().c_str()
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
            this->drawTreeBranch(child, i);
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
