#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/operators.hpp>
#include <Geode/utils/platform.hpp>
#include "../DevTools.hpp"

USE_GEODE_NAMESPACE();

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
    if (contentSize != node->getContentSize())
        node->setContentSize(contentSize);

    int zOrder = node->getZOrder();
    ImGui::InputInt("Z Order", &zOrder);
    if (node->getZOrder() != zOrder)
        node->setZOrder(zOrder);
    
    auto visible = node->isVisible();
    ImGui::Checkbox("Visible", &visible);
    if (visible != node->isVisible())
        node->setVisible(visible);

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
        auto labelStr = labelNode->getString();
        char text[256];
        strcpy_s(text, labelStr);
        ImGui::InputText("Text", text, 256);
        if (strcmp(text, labelStr)) {
            labelNode->setString(text);
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
