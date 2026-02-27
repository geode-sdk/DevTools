#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

std::string formatNodeName(CCNode* node, int prio) {
    std::string name = fmt::format("[{}] {} ", prio, geode::cocos::getObjectName(node));
    if (node->getTag() != -1) {
        name += fmt::format("({}) ", node->getTag());
    }
    if (node->getID().size()) {
        name += fmt::format("\"{}\" ", node->getID());
    }
    return name;
}

void DevTools::drawPrioHandler(CCTouchHandler* handler) {
    auto node = typeinfo_cast<CCNode*>(handler->getDelegate());
    if (node) {
        bool selected = DevTools::get()->getSelectedNode() == node;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
        if (selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        const auto name = formatNodeName(node, handler->getPriority());
        bool expanded = ImGui::TreeNodeEx(node, flags, "%s", name.c_str());

        if (ImGui::IsItemClicked()) {
            DevTools::get()->selectNode(node);
            selected = true;

            CCNode* parent = node->getParent();
            while (parent) {
                m_nodeOpen[parent] = true;
                parent = parent->getParent();
            }
        }
        if (expanded) {
            ImGui::TreePop();
        }
    }
}

void DevTools::drawPrioTree() {
#ifdef GEODE_IS_MOBILE
    ImGui::Dummy({0.f, 60.f});
#endif
    ImGui::Text("Standard Handlers");

    for (auto d : CCTouchDispatcher::get()->m_pStandardHandlers->asExt<CCTouchHandler>()) {
        drawPrioHandler(d);
    }

    ImGui::Text("Targeted Handlers");

    for (auto d : CCTouchDispatcher::get()->m_pTargetedHandlers->asExt<CCTouchHandler>()) {
        drawPrioHandler(d);
    }
}