#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

USE_GEODE_NAMESPACE();

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
    ImGui::Text("Address: 0x%p", node);
    ImGui::SameLine();
    if (ImGui::Button(U8STR(FEATHER_COPY " Copy"))) {
        clipboard::write(
            CCString::createWithFormat(
                "%X", reinterpret_cast<uintptr_t>(node)
            )->getCString()
        );
    }
    if (node->getUserData()) {
        ImGui::Text("User data: 0x%p", node->getUserData());
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
    
    if (auto rgbaNode = dynamic_cast<CCRGBAProtocol*>(node)) {
        auto color = rgbaNode->getColor();
        float _color[4] = { color.r / 255.f, color.g / 255.f, color.b / 255.f, rgbaNode->getOpacity() / 255.f };
        ImGui::ColorEdit4("Color", _color);
        rgbaNode->setColor({
            static_cast<GLubyte>(_color[0] * 255),
            static_cast<GLubyte>(_color[1] * 255),
            static_cast<GLubyte>(_color[2] * 255)
        });
        rgbaNode->setOpacity(static_cast<GLubyte>(_color[3] * 255));
    }
    
    if (auto labelNode = dynamic_cast<CCLabelProtocol*>(node)) {
        std::string str = labelNode->getString();
        if (ImGui::InputText("Text", &str, 256)) {
            labelNode->setString(str.c_str());
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

            if (autoScale == 0) {
                ImGui::BeginDisabled();
            }
            if (checkbox("Break Line", opts, AXIS_GET(BreakLine))) {
                updateLayout = true;
            }
            if (checkbox("Same Line", opts, AXIS_GET(SameLine))) {
                updateLayout = true;
            }
            if (autoScale == 0) {
                ImGui::EndDisabled();
            }

            auto prio = opts->getScalePriority();
            if (ImGui::DragInt("Scale Priority", &prio)) {
                opts->setScalePriority(prio);
                updateLayout = true;
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
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add Test Child"))) {
            auto spr = CCSprite::create("GJ_button_01.png");
            auto btn = CCMenuItemSpriteExtra::create(spr, node, nullptr);
            node->addChild(btn);
            node->updateLayout();
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
                if (updateAlign) {
                    layout->setCrossAxisAlignment(static_cast<AxisAlignment>(align));
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
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AxisLayout"))) {
            node->setLayout(AxisLayout::create());
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
