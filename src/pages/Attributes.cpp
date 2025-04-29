#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "../platform/utils.hpp"

using namespace geode::prelude;

#define AXIS_GET(Name_) \
    &AxisLayoutOptions::get##Name_, \
    &AxisLayoutOptions::set##Name_

template <class T, class R>
bool checkbox(const char* text, T* ptr, bool(T::* get)(), R(T::* set)(bool)) {
    bool value = (ptr->*get)();
    if (ImGui::Checkbox(text, &value)) {
        (ptr->*set)(value);
        return true;
    }
    return false;
}

template <class T, class R>
bool checkbox(const char* text, T* ptr, bool(T::* get)() const, R(T::* set)(bool)) {
    bool value = (ptr->*get)();
    if (ImGui::Checkbox(text, &value)) {
        (ptr->*set)(value);
        return true;
    }
    return false;
}

void DevTools::drawNodeAttributes(CCNode* node) {
    if (ImGui::Button("Deselect")) {
        return this->selectNode(nullptr);
    }
    ImGui::Text("Address: %s", fmt::to_string(fmt::ptr(node)).c_str());
    ImGui::SameLine();
    if (ImGui::Button(U8STR(FEATHER_COPY " Copy"))) {
        clipboard::write(
            utils::intToHex(reinterpret_cast<uintptr_t>(node))
        );
    }
    if (node->getUserData()) {
        ImGui::Text("User data: 0x%p", node->getUserData());
    }

    if (!node->getID().empty()) {
        std::string nodeID = node->getID();
        ImGui::Text("Node ID: %s", nodeID.c_str());
        ImGui::SameLine();
        if (ImGui::Button(U8STR(FEATHER_COPY " Copy##copynodeid"))) {
            clipboard::write(nodeID);
        }
    } else {
        ImGui::Text("Node ID: N/A");
    }

    if (auto menuItemNode = typeinfo_cast<CCMenuItem*>(node)) {
        const auto selector = menuItemNode->m_pfnSelector;
        if (!selector) {
            std::string addr = "N/A";
            ImGui::Text("CCMenuItem selector: %s", addr.c_str());
        } else {
            const auto addr = formatAddressIntoOffset(addresser::getNonVirtual(selector), true);
            ImGui::Text("CCMenuItem selector: %s", addr.c_str());
            ImGui::SameLine();
            if (ImGui::Button(U8STR(FEATHER_COPY " Copy##copymenuitem"))) {
                const auto addrNoModule = formatAddressIntoOffset(addresser::getNonVirtual(selector), false);
                clipboard::write(addrNoModule);
            }
        }
    }

    float pos[2] = {
        node->getPositionX(),
        node->getPositionY()
    };
    ImGui::DragFloat2("Position", pos);
    node->setPosition(pos[0], pos[1]);

    float scale[3] = { node->getScale(), node->getScaleX(), node->getScaleY() };
    ImGui::DragFloat3("Scale", scale, 0.025f);
    if (node->getScale() != scale[0]) {
        node->setScale(scale[0]);
    } else {
        node->setScaleX(scale[1]);
        node->setScaleY(scale[2]);
    }

    float rot[3] = { node->getRotation(), node->getRotationX(), node->getRotationY() };
    ImGui::DragFloat3("Rotation", rot);
    if (node->getRotation() != rot[0]) {
        node->setRotation(rot[0]);
    } else {
        node->setRotationX(rot[1]);
        node->setRotationY(rot[2]);
    }

    float _skew[2] = { node->getSkewX(), node->getSkewY() };
    ImGui::DragFloat2("Skew", _skew);
    node->setSkewX(_skew[0]);
    node->setSkewY(_skew[1]);

    auto anchor = node->getAnchorPoint();
    ImGui::DragFloat2("Anchor Point", &anchor.x, 0.05f, 0.f, 1.f);
    node->setAnchorPoint(anchor);

    auto contentSize = node->getContentSize();
    ImGui::DragFloat2("Content Size", &contentSize.width);
    if (contentSize != node->getContentSize()) {
        node->setContentSize(contentSize);
        node->updateLayout();
    }

    int zOrder = node->getZOrder();
    ImGui::InputInt("Z Order", &zOrder);
    if (node->getZOrder() != zOrder) {
        node->setZOrder(zOrder);
    }
    
    checkbox("Visible", node, &CCNode::isVisible, &CCNode::setVisible);
    checkbox(
        "Ignore Anchor Point for Position",
        node,
        &CCNode::isIgnoreAnchorPointForPosition,
        &CCNode::ignoreAnchorPointForPosition
    );
    
    if (auto rgbaNode = typeinfo_cast<CCRGBAProtocol*>(node)) {
        auto color = rgbaNode->getColor();
        float _color[4] = { color.r / 255.f, color.g / 255.f, color.b / 255.f, rgbaNode->getOpacity() / 255.f };
        if (ImGui::ColorEdit4("Color", _color)) {
            rgbaNode->setColor(ccColor3B{
                static_cast<GLubyte>(_color[0] * 255),
                static_cast<GLubyte>(_color[1] * 255),
                static_cast<GLubyte>(_color[2] * 255)
            });

            rgbaNode->setOpacity(static_cast<GLubyte>(_color[3] * 255));
        }
    }
    
    if (auto labelNode = typeinfo_cast<CCLabelProtocol*>(node)) {
        std::string str = labelNode->getString();
        if (ImGui::InputText("Text", &str, 256)) {
            labelNode->setString(str.c_str());
        }
    }

    if (auto textureProtocol = typeinfo_cast<CCTextureProtocol*>(node)) {
        if (auto texture = textureProtocol->getTexture()) {
            auto* cachedTextures = CCTextureCache::sharedTextureCache()->m_pTextures;
            for (auto [key, obj] : CCDictionaryExt<std::string, CCTexture2D*>(cachedTextures)) {
                if (obj == texture) {
                    ImGui::TextWrapped("Texture name: %s", key.c_str());
                    break;
                }
            }

            if (auto spriteNode = typeinfo_cast<CCSprite*>(node)) {
                auto* cachedFrames = CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
                const auto rect = spriteNode->getTextureRect();
                for (auto [key, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(cachedFrames)) {
                    if (frame->getTexture() == texture && frame->getRect() == rect) {
                        ImGui::Text("Frame name: %s", key.c_str());
                        ImGui::SameLine();
                        if (ImGui::Button(U8STR(FEATHER_COPY " Copy##copysprframename"))) {
                            clipboard::write(key);
                        }
                        break;
                    }
                }
            }

        }
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();
    
    if (auto rawOpts = node->getLayoutOptions()) {
        ImGui::Text("Layout options: %s", typeid(*rawOpts).name());

        if (ImGui::Button(U8STR(FEATHER_REFRESH_CW " Update Parent Layout"))) {
            if (auto parent = node->getParent()) {
                parent->updateLayout();
            }
        }
        if (auto opts = typeinfo_cast<AxisLayoutOptions*>(rawOpts)) {
            bool updateLayout = false;

            ImGui::Text("Auto Scale");
            auto updateAxis = false;
            int autoScale = opts->getAutoScale() ? opts->getAutoScale().value() + 1 : 0;
            updateAxis |= ImGui::RadioButton("Default", &autoScale, 0);
            ImGui::SameLine();
            updateAxis |= ImGui::RadioButton("Enable", &autoScale, 1);
            ImGui::SameLine();
            updateAxis |= ImGui::RadioButton("Disable", &autoScale, 2);
            if (updateAxis) {
                switch (autoScale) {
                    case 0: opts->setAutoScale(std::nullopt); break;
                    case 1: opts->setAutoScale(true); break;
                    case 2: opts->setAutoScale(false); break;
                }
                updateLayout = true;
            }

            if (checkbox("Break Line", opts, AXIS_GET(BreakLine))) {
                updateLayout = true;
            }
            if (checkbox("Same Line", opts, AXIS_GET(SameLine))) {
                updateLayout = true;
            }

            auto prio = opts->getScalePriority();
            if (ImGui::DragInt("Scale Priority", &prio, .03f)) {
                opts->setScalePriority(prio);
                updateLayout = true;
            }

            if (updateLayout && node->getParent()) {
                node->getParent()->updateLayout();
            }
        }
        else if (auto opts = typeinfo_cast<AnchorLayoutOptions*>(rawOpts)) {
            bool updateLayout = false;

            auto offset = opts->getOffset();
            ImGui::DragFloat2("Offset", &offset.x);
            if (opts->getOffset() != offset) {
                opts->setOffset(offset);
                updateLayout = true;
            }

            auto anchor = static_cast<int>(opts->getAnchor());
            auto updateAnchor = false;
            ImGui::BeginTable("anchor-table", 3);
            ImGui::TableNextColumn();
            updateAnchor |= ImGui::RadioButton("Top Left", &anchor, static_cast<int>(Anchor::TopLeft));
            updateAnchor |= ImGui::RadioButton("Left", &anchor, static_cast<int>(Anchor::Left));
            updateAnchor |= ImGui::RadioButton("Bottom Left", &anchor, static_cast<int>(Anchor::BottomLeft));
            ImGui::TableNextColumn();
            updateAnchor |= ImGui::RadioButton("Top", &anchor, static_cast<int>(Anchor::Top));
            updateAnchor |= ImGui::RadioButton("Center", &anchor, static_cast<int>(Anchor::Center));
            updateAnchor |= ImGui::RadioButton("Bottom", &anchor, static_cast<int>(Anchor::Bottom));
            ImGui::TableNextColumn();
            updateAnchor |= ImGui::RadioButton("Top Right", &anchor, static_cast<int>(Anchor::TopRight));
            updateAnchor |= ImGui::RadioButton("Right", &anchor, static_cast<int>(Anchor::Right));
            updateAnchor |= ImGui::RadioButton("Bottom Right", &anchor, static_cast<int>(Anchor::BottomRight));
            ImGui::EndTable();

            if (updateAnchor) {
                if (opts->getAnchor() != static_cast<Anchor>(anchor)) {
                    opts->setAnchor(static_cast<Anchor>(anchor));
                    updateLayout = true;
                }
            }

            if (updateLayout && node->getParent()) {
                node->getParent()->updateLayout();
            }
        }
    }
    else {
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AxisLayoutOptions"))) {
            node->setLayoutOptions(AxisLayoutOptions::create());
        }
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AnchorLayoutOptions"))) {
            node->setLayoutOptions(AnchorLayoutOptions::create());
        }
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    if (auto delegate = typeinfo_cast<CCTouchDelegate*>(node)) {
        if (auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
            auto priority = handler->getPriority();

            if (ImGui::DragInt("Touch Priority", &priority, .03f)) {
                CCTouchDispatcher::get()->setPriority(priority, handler->getDelegate());
            }
        }
    }
    

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    if (auto rawLayout = node->getLayout()) {
        ImGui::Text("Layout: %s", typeid(*rawLayout).name());
        
        if (ImGui::Button(U8STR(FEATHER_REFRESH_CW " Update Layout"))) {
            node->updateLayout();
        }
        ImGui::SameLine();
        if (ImGui::Button(U8STR(FEATHER_TRASH_2 " Remove Layout"))) {
            node->setLayout(nullptr);
        }
        ImGui::SameLine();
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add Test Child"))) {
            auto spr = CCSprite::create("GJ_button_01.png");
            auto btn = CCMenuItemSpriteExtra::create(spr, node, nullptr);
            node->addChild(btn);
            node->updateLayout();
        }
        if (auto layout = typeinfo_cast<SimpleAxisLayout*>(rawLayout)) {
            bool updateLayout = false;
            auto axis = static_cast<int>(layout->getAxis());
            ImGui::Text("Axis");
            auto updateAxis = false;
            updateAxis |= ImGui::RadioButton("Row",    &axis, static_cast<int>(Axis::Row));
            ImGui::SameLine();
            updateAxis |= ImGui::RadioButton("Column", &axis, static_cast<int>(Axis::Column));
            if (updateAxis) {
                if (layout->getAxis() != static_cast<Axis>(axis)) {
                    node->setContentSize({
                        node->getContentSize().height,
                        node->getContentSize().width
                    });
                }
                layout->setAxis(static_cast<Axis>(axis));
                updateLayout = true;
            }
            {
                auto axisScaling = static_cast<int>(layout->getMainAxisScaling());
                ImGui::Text("Main Axis Scaling");
                bool updateScaling = false;
                updateScaling |= ImGui::RadioButton(
                    "None", &axisScaling, static_cast<int>(AxisScaling::None)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Scale Down", &axisScaling, static_cast<int>(AxisScaling::ScaleDown)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Scale", &axisScaling, static_cast<int>(AxisScaling::Scale)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Grow", &axisScaling, static_cast<int>(AxisScaling::Grow)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Fit", &axisScaling, static_cast<int>(AxisScaling::Fit)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Scale Down Gaps", &axisScaling, static_cast<int>(AxisScaling::ScaleDownGaps)
                );
                if (updateScaling) {
                    layout->setMainAxisScaling(static_cast<AxisScaling>(axisScaling));
                    updateLayout = true;
                }
            }
            {
                auto axisScaling = static_cast<int>(layout->getCrossAxisScaling());
                ImGui::Text("Cross Axis Scaling");
                bool updateScaling = false;
                updateScaling |= ImGui::RadioButton(
                    "None##crossscale0", &axisScaling, static_cast<int>(AxisScaling::None)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Scale Down##crossscale1", &axisScaling, static_cast<int>(AxisScaling::ScaleDown)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Scale##crossscale2", &axisScaling, static_cast<int>(AxisScaling::Scale)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Grow##crossscale3", &axisScaling, static_cast<int>(AxisScaling::Grow)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Fit##crossscale4", &axisScaling, static_cast<int>(AxisScaling::Fit)
                );
                ImGui::SameLine();
                updateScaling |= ImGui::RadioButton(
                    "Scale Down Gaps##crossscale5", &axisScaling, static_cast<int>(AxisScaling::ScaleDownGaps)
                );
                if (updateScaling) {
                    layout->setCrossAxisScaling(static_cast<AxisScaling>(axisScaling));
                    updateLayout = true;
                }
            }
            {
                auto align = static_cast<int>(layout->getMainAxisAlignment());
                ImGui::Text("Main Axis Alignment");
                bool updateAlign = false;
                updateAlign |= ImGui::RadioButton(
                    "Start", &align, static_cast<int>(MainAxisAlignment::Start)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Center", &align, static_cast<int>(MainAxisAlignment::Center)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "End", &align, static_cast<int>(MainAxisAlignment::End)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Even", &align, static_cast<int>(MainAxisAlignment::Even)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Between", &align, static_cast<int>(MainAxisAlignment::Between)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Around", &align, static_cast<int>(MainAxisAlignment::Around)
                );
                if (updateAlign) {
                    layout->setMainAxisAlignment(static_cast<MainAxisAlignment>(align));
                    updateLayout = true;
                }
            }
            {
                auto align = static_cast<int>(layout->getCrossAxisAlignment());
                ImGui::Text("Cross Axis Alignment");
                bool updateAlign = false;
                updateAlign |= ImGui::RadioButton(
                    "Start##crossalign0", &align, static_cast<int>(CrossAxisAlignment::Start)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Center##crossalign1", &align, static_cast<int>(CrossAxisAlignment::Center)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "End##crossalign2", &align, static_cast<int>(CrossAxisAlignment::End)
                );
                if (updateAlign) {
                    layout->setCrossAxisAlignment(static_cast<CrossAxisAlignment>(align));
                    updateLayout = true;
                }
            }

            std::string axisDirection = axis ? "Bottom to Top" : "Left to Right";
            std::string secondAxisDirection = axis ? "Top to Bottom" : "Right to Left";

            std::string crossAxisDirection = !axis ? "Bottom to Top" : "Left to Right";
            std::string secondcrossAxisDirection = !axis ? "Top to Bottom" : "Right to Left";
            {
                auto direction = static_cast<int>(layout->getMainAxisDirection());
                ImGui::Text("Main Axis Direction");
                bool updateDirection = false;
                
                updateDirection |= ImGui::RadioButton(
                    axisDirection.c_str(), &direction, static_cast<int>(AxisDirection::FrontToBack)
                );
                ImGui::SameLine();
                updateDirection |= ImGui::RadioButton(
                    secondAxisDirection.c_str(), &direction, static_cast<int>(AxisDirection::BackToFront)
                );
                if (updateDirection) {
                    layout->setMainAxisDirection(static_cast<AxisDirection>(direction));
                    updateLayout = true;
                }
            }
            {
                auto direction = static_cast<int>(layout->getCrossAxisDirection());
                ImGui::Text("Cross Axis Direction");
                bool updateDirection = false;
                updateDirection |= ImGui::RadioButton(
                    crossAxisDirection.c_str(), &direction, static_cast<int>(AxisDirection::FrontToBack)
                );
                ImGui::SameLine();
                updateDirection |= ImGui::RadioButton(
                    secondcrossAxisDirection.c_str(), &direction, static_cast<int>(AxisDirection::BackToFront)
                );
                if (updateDirection) {
                    layout->setCrossAxisDirection(static_cast<AxisDirection>(direction));
                    updateLayout = true;
                }
            }

            auto gap = layout->getGap();
            if (ImGui::DragFloat("Gap", &gap)) {
                layout->setGap(gap);
                updateLayout = true;
            }

            auto minRelativeScaleOpt = layout->getMinRelativeScale();
            float minRelativeScale = minRelativeScaleOpt.value_or(0);
            bool hasMinRelativeScale = minRelativeScaleOpt.has_value();

            if (ImGui::Checkbox("Has Min Relative Scale", &hasMinRelativeScale)) {
                if (hasMinRelativeScale) {
                    layout->setMinRelativeScale(0);
                }
                else {
                    layout->setMinRelativeScale(std::nullopt);
                }
                updateLayout = true;
            }

            if (ImGui::DragFloat("Min Relative Scale", &minRelativeScale)) {
                if (hasMinRelativeScale) {
                    layout->setMinRelativeScale(minRelativeScale);
                    updateLayout = true;
                }
            }

            auto maxRelativeScaleOpt = layout->getMaxRelativeScale();
            float maxRelativeScale = maxRelativeScaleOpt.value_or(0);
            bool hasMaxRelativeScale = maxRelativeScaleOpt.has_value();
            if (ImGui::Checkbox("Has Max Relative Scale", &hasMaxRelativeScale)) {
                if (hasMaxRelativeScale) {
                    layout->setMaxRelativeScale(0);
                }
                else {
                    layout->setMaxRelativeScale(std::nullopt);
                }
                updateLayout = true;
            }

            if (ImGui::DragFloat("Max Relative Scale", &maxRelativeScale)) {
                if (hasMaxRelativeScale) {
                    layout->setMaxRelativeScale(maxRelativeScale);
                    updateLayout = true;
                }
            }

            if (updateLayout) {
                node->updateLayout();
            }
        }
        if (auto layout = typeinfo_cast<AxisLayout*>(rawLayout)) {
            bool updateLayout = false;

            auto axis = static_cast<int>(layout->getAxis());
            ImGui::Text("Axis");
            auto updateAxis = false;
            updateAxis |= ImGui::RadioButton("Row",    &axis, static_cast<int>(Axis::Row));
            ImGui::SameLine();
            updateAxis |= ImGui::RadioButton("Column", &axis, static_cast<int>(Axis::Column));
            if (updateAxis) {
                if (layout->getAxis() != static_cast<Axis>(axis)) {
                    node->setContentSize({
                        node->getContentSize().height,
                        node->getContentSize().width
                    });
                }
                layout->setAxis(static_cast<Axis>(axis));
                updateLayout = true;
            }

            auto axisReverse = layout->getAxisReverse();
            if (ImGui::Checkbox("Flip Axis Direction", &axisReverse)) {
                layout->setAxisReverse(axisReverse);
                updateLayout = true;
            }
            axisReverse = layout->getCrossAxisReverse();
            if (ImGui::Checkbox("Flip Cross Axis Direction", &axisReverse)) {
                layout->setCrossAxisReverse(axisReverse);
                updateLayout = true;
            }

            {
                auto align = static_cast<int>(layout->getAxisAlignment());
                ImGui::Text("Axis Alignment");
                bool updateAlign = false;
                updateAlign |= ImGui::RadioButton(
                    "Start", &align, static_cast<int>(AxisAlignment::Start)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Center", &align, static_cast<int>(AxisAlignment::Center)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "End", &align, static_cast<int>(AxisAlignment::End)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Even", &align, static_cast<int>(AxisAlignment::Even)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Between", &align, static_cast<int>(AxisAlignment::Between)
                );
                if (updateAlign) {
                    layout->setAxisAlignment(static_cast<AxisAlignment>(align));
                    updateLayout = true;
                }
            }

            {
                auto align = static_cast<int>(layout->getCrossAxisAlignment());
                ImGui::Text("Cross Axis Alignment");
                bool updateAlign = false;
                updateAlign |= ImGui::RadioButton(
                    "Start##cross0", &align, static_cast<int>(AxisAlignment::Start)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Center##cross1", &align, static_cast<int>(AxisAlignment::Center)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "End##cross2", &align, static_cast<int>(AxisAlignment::End)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Even##cross3", &align, static_cast<int>(AxisAlignment::Even)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Between##cross4", &align, static_cast<int>(AxisAlignment::Between)
                );
                if (updateAlign) {
                    layout->setCrossAxisAlignment(static_cast<AxisAlignment>(align));
                    updateLayout = true;
                }
            }

            {
                auto align = static_cast<int>(layout->getCrossAxisLineAlignment());
                ImGui::Text("Cross Axis Line Alignment");
                bool updateAlign = false;
                updateAlign |= ImGui::RadioButton(
                    "Start##crossline0", &align, static_cast<int>(AxisAlignment::Start)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Center##crossline1", &align, static_cast<int>(AxisAlignment::Center)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "End##crossline2", &align, static_cast<int>(AxisAlignment::End)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Even##crossline3", &align, static_cast<int>(AxisAlignment::Even)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Between##crossline4", &align, static_cast<int>(AxisAlignment::Between)
                );
                if (updateAlign) {
                    layout->setCrossAxisLineAlignment(static_cast<AxisAlignment>(align));
                    updateLayout = true;
                }
            }

            auto gap = layout->getGap();
            if (ImGui::DragFloat("Gap", &gap)) {
                layout->setGap(gap);
                updateLayout = true;
            }

            auto autoScale = layout->getAutoScale();
            if (ImGui::Checkbox("Auto Scale", &autoScale)) {
                layout->setAutoScale(autoScale);
                updateLayout = true;
            }

            auto grow = layout->getGrowCrossAxis();
            if (ImGui::Checkbox("Grow Cross Axis", &grow)) {
                layout->setGrowCrossAxis(grow);
                updateLayout = true;
            }

            auto overflow = layout->getCrossAxisOverflow();
            if (ImGui::Checkbox("Allow Cross Axis Overflow", &overflow)) {
                layout->setCrossAxisOverflow(overflow);
                updateLayout = true;
            }

            if (updateLayout) {
                node->updateLayout();
            }
        }
    }
    else {
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add SimpleAxisLayout"))) {
            node->setLayout(SimpleAxisLayout::create(Axis::Row));
        }
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AxisLayout"))) {
            node->setLayout(AxisLayout::create());
        }
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AnchorLayout"))) {
            node->setLayout(AnchorLayout::create());
        }
    }
}

void DevTools::drawAttributes() {
    if (!m_selectedNode) {
        ImGui::TextWrapped("Select a Node to Edit in the Scene or Tree");
    } else {
        this->drawNodeAttributes(m_selectedNode);
    }
}
