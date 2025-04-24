#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include "../utils.hpp"
#ifndef GEODE_IS_WINDOWS
#include <cxxabi.h>
#endif

using namespace geode::prelude;

std::string getNodeName(CCObject* node) {
#ifdef GEODE_IS_WINDOWS
    std::string_view sv = typeid(*node).name();
    if (sv.starts_with("class ")) sv.remove_prefix(6);
    if (sv.starts_with("struct ")) sv.remove_prefix(7);

    return std::string(sv);
#else
    std::string ret;

    int status = 0;
    auto demangle = abi::__cxa_demangle(typeid(*node).name(), 0, 0, &status);
    if (status == 0) {
        ret = demangle;
    }
    free(demangle);

    return ret;
#endif
}

void DevTools::drawTreeBranch(CCNode* node, size_t index, const std::vector<CCNode*>& selectedAncestors) {
    bool isSelectedAncestor = std::find(selectedAncestors.begin(), selectedAncestors.end(), node) != selectedAncestors.end();

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
    if (isSelectedAncestor) {
        ImGui::SetNextItemOpen(true);
    }
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
            this->drawTreeBranch(child, i++, selectedAncestors);
        }
        ImGui::TreePop();
    }
}

void DevTools::drawTreeHoverSelectButton() {
    auto window = ImGui::GetCurrentWindow();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    ImGui::SetNextWindowPos(ImVec2(
        windowPos.x + windowSize.x - 35.f - (window->ScrollbarY ? 15.f : 0.f),
        windowPos.y + 35.f
    ));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::BeginChild("TreeHoverBtn", ImVec2(25.f, 25.f), false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    if (ImGui::Button(U8STR(FEATHER_EXTERNAL_LINK), ImVec2(25.f, 25.f))) {
        this->m_nodeHoverSelectEnabled ^= 1;
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Select a node by hovering and clicking on it");
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

static void doHighlight(CCNode* node) {
    DevTools::get()->highlightNode(node, HighlightMode::Hovered);
}

static CCRect getActualMenuRect(CCNode* menu) {
    // returns local rect indicating node coverage of this ccmenu
    if (menu->getChildrenCount() == 0) {
        return CCRect{};
    }

    CCPoint bottomLeft{};
    CCPoint topRight{};

    for (auto node : CCArrayExt<CCNode>(menu->getChildren())) {
        auto bbox = node->boundingBox();

        if (bbox.origin.x < bottomLeft.x) {
            bottomLeft.x = bbox.origin.x;
        }

        if (bbox.origin.y < bottomLeft.y) {
            bottomLeft.y = bbox.origin.y;
        }

        if (bbox.getMaxX() > topRight.x) {
            topRight.x = bbox.getMaxX();
        }

        if (bbox.getMaxY() > topRight.y) {
            topRight.y = bbox.getMaxY();
        }
    }

    return CCRect(bottomLeft, topRight - bottomLeft);
}

static void recurseFindNode(std::vector<CCNode*>& candidates, const CCPoint& mousePos, CCNode* node) {
    if (!node->isVisible()) return;

    auto winSize = CCDirector::get()->getWinSize();

	auto bbox = node->boundingBox();
    auto parent = node->getParent();

    if (parent) {
        bbox.origin *= parent->getScale();
        bbox += parent->convertToWorldSpace(CCPoint{});
    }

    if (auto menu = typeinfo_cast<CCMenu*>(node)) {
        // layoutless ccmenus usually have dumb sizes and positions, let's fix that
        CCRect cov = getActualMenuRect(menu);

        bbox.origin += cov.origin;
        bbox.size = cov.size;
    } else if (node->getContentSize() == CCSize{} || node->getContentSize() == winSize) {
        CCRect cov = getActualMenuRect(node);

        bbox.origin += cov.origin;
        bbox.size = cov.size;
    }

    if (!bbox.containsPoint(mousePos)) {
        return;
    }

    candidates.push_back(node);

    // no one would really want to select the individual characters in a label or other batch node
    if (!typeinfo_cast<CCSpriteBatchNode*>(node)) {
        for (auto child : CCArrayExt<CCNode>(node->getChildren())) {
            recurseFindNode(candidates, mousePos, child);
        }
    }
}

static std::string formatNodeVec(const std::vector<CCNode*>& nodes) {
    std::string out;
    bool first = true;

    for (auto node : nodes) {
        if (first) {
            first = false;
        } else {
            out += ", ";
        }

        auto id = node->getID();

        if (id.empty()) {
            out += fmt::format("{} @ {}", getNodeName(node), (void*)node);
        } else {
            out += fmt::format("{} ({}) @ {}", getNodeName(node), id, (void*)node);
        }
    }

    return out;
}

void DevTools::drawHoveredNodeHighlight() {
    auto mousePos = convertTouch(getMousePos());
    if (mousePos == CCPoint{0.f, 0.f}) {
        return;
    }

    std::vector<CCNode*> candidates;

    auto scene = CCScene::get();
    if (scene) {
        recurseFindNode(candidates, mousePos, scene);
    }

    if (candidates.empty()) return;

    // 1. remove all nodes that already have a child in the vector
    auto removeParents = [&] {
        size_t pre = candidates.size();

        for (size_t i = 0; i < candidates.size(); i++) {
            auto node = candidates[i];
            auto parent = node->getParent();

            if (!parent) continue;

            // remove the parent if found
            auto it = std::find(candidates.begin(), candidates.end(), parent);
            if (it != candidates.end()) {
                candidates.erase(it);
                i--; // prevent over increment
            }
        }

        return candidates.size() != pre;
    };

    // do multiple passes
    while (removeParents());

    // 2. remove some annoying nodes

    std::erase_if(candidates, [](CCNode* node) {
        return typeinfo_cast<CCMenu*>(node) || typeinfo_cast<CCClippingNode*>(node);
    });

    // 3. if multiple candidates remain, compare in pairs, for each pair
    // traverse parent chain until a mutual parent is found, then select the one that should be drawn above
    if (candidates.empty()) {
        return;
    } else if (candidates.size() == 1) {
        doHighlight(candidates[0]);
        return;
    }

    std::vector<CCNode*> parentChain;

    while (candidates.size() > 1) {
        parentChain.clear();

        auto node1 = candidates[candidates.size() - 1];
        auto node2 = candidates[candidates.size() - 2];

        auto parent1 = node1;
        while ((parent1 = parent1->getParent())) {
            parentChain.push_back(parent1);
        }

        // traverse parent chain for node 2
        auto parent2 = node2;
        CCNode* mutualParent = nullptr;
        while ((parent2 = parent2->getParent())) {
            auto it = std::find(parentChain.begin(), parentChain.end(), parent2);
            if (it != parentChain.end()) {
                mutualParent = *it;
                break;
            }
        }

        if (!mutualParent) {
            log::error("No mutual parent for nodes: {} and {}, parent chain of node1: {}", node1, node2, formatNodeVec(parentChain));
            return;
        }

        // find the child of parent2 that leads us to the node
        auto findParentChild = [&](CCNode* superparent, CCNode* child) {
            auto p = child->getParent();

            if (p == superparent) {
                return child;
            }

            while (p && p->getParent() != superparent) {
                p = p->getParent();
            }

            return p;
        };

        auto ch1 = findParentChild(mutualParent, node1);
        auto ch2 = findParentChild(mutualParent, node2);

        int z1 = ch1->getZOrder();
        int z2 = ch2->getZOrder();

        bool drawSecond = false;

        if (z2 > z1) {
            drawSecond = true;
        } else if (z2 < z1) {
            drawSecond = false;
        } else {
            drawSecond = ch2->m_uOrderOfArrival > ch1->m_uOrderOfArrival;
        }

        if (drawSecond) {
            candidates.pop_back();
        } else {
            candidates.erase(candidates.end() - 2);
        }
    }

    doHighlight(candidates[0]);

    if (m_nodeHoverSelectConsumeTouch) {
        m_nodeHoverSelectConsumeTouch = false;
        m_nodeHoverSelectEnabled = false;

        DevTools::get()->selectNode(candidates[0]);
    }
}

void DevTools::drawTree() {
#ifdef GEODE_IS_MOBILE
    ImGui::Dummy({0.f, 60.f});
#endif

    std::vector<CCNode*> selectedAncestors;
    if (m_selectedNode) {
        auto parent = m_selectedNode;

        while ((parent = parent->getParent())) {
            selectedAncestors.push_back(parent);
        }
    }

    this->drawTreeBranch(CCDirector::get()->getRunningScene(), 0, selectedAncestors);
    this->drawTreeHoverSelectButton();

    if (m_nodeHoverSelectEnabled) {
        this->drawHoveredNodeHighlight();
    }
}
