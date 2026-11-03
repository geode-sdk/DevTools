#include "platform/platform.hpp"
#include <Geode/modify/AchievementNotifier.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCNode.hpp>
#include <Geode/modify/GameToolbox.hpp>
#include "DevTools.hpp"
#include <imgui.h>
#include "nodes/DragButton.hpp"
#include <imgui-cocos.hpp>

using namespace geode::prelude;

class $modify(CCNode) {
    void sortAllChildren() override {
        if (DevTools::get()->shouldOrderChildren()) {
            CCNode::sortAllChildren();
        }
    }
};

#include <Geode/loader/GameEvent.hpp>
$execute {
    GameEvent(GameEventType::Loaded).listen([] {
        if (DevTools::get()->isButtonEnabled()) DevTools::get()->setupDragButton();
    }).leak();

    listenForKeybindSettingPresses("open-bind", [](Keybind const& keybind, bool down, bool repeat, double timestamp) {
        if (down && !repeat) {
            DevTools::get()->toggle();
        }
    });
}

#include <Geode/modify/CCScene.hpp>
class $modify(CCScene) {
    int getHighestChildZ() {
        auto btn = DevTools::get()->getDragButton();
        if (!btn) return CCScene::getHighestChildZ();
        int z = btn->getZOrder();
        btn->setZOrder(-1);
        int ret = CCScene::getHighestChildZ();
        btn->setZOrder(z);
        return ret;
    }
};

class $modify(GameToolbox) {
    static void preVisitWithClippingRect(CCNode* node, CCRect clipRect) {
        if (!node->isVisible() || !DevTools::get()->isVisible())
            return GameToolbox::preVisitWithClippingRect(node, clipRect);

        glEnable(GL_SCISSOR_TEST);

        clipRect.origin = node->convertToWorldSpace(clipRect.origin);

        kmMat4 mat;
        kmGLGetMatrix(KM_GL_PROJECTION, &mat);
        if (mat.mat[5] < 0) {
            auto ws = CCDirector::get()->getWinSize();
            clipRect.origin.y = ws.height - (clipRect.origin.y + node->getContentSize().height);
        }

        CCEGLView::get()->setScissorInPoints(
            clipRect.origin.x,
            clipRect.origin.y,
            clipRect.size.width,
            clipRect.size.height
        );
    }

};

static GLRenderCtx* s_gdTexture = nullptr;

class $modify(CCDirector) {
    void willSwitchToScene(CCScene* scene) {
        CCDirector::willSwitchToScene(scene);
        DevTools::get()->sceneChanged();
    }

    void drawScene() {
        if (!DevTools::get()->shouldUseGDWindow()) {
            return CCDirector::drawScene();
        }

        if (!DevTools::get()->shouldPopGame()) {
            if (s_gdTexture) {
                delete s_gdTexture;
                s_gdTexture = nullptr;
            }
            shouldPassEventsToGDButTransformed() = false;
            CCDirector::drawScene();
            return;
        }

        if (shouldUpdateGDRenderBuffer()) {
            if (s_gdTexture) {
                delete s_gdTexture;
                s_gdTexture = nullptr;
            }
            shouldUpdateGDRenderBuffer() = false;
        }

        auto winSize = this->getOpenGLView()->getViewPortRect() * geode::utils::getDisplayFactor();
        if (!s_gdTexture) {
            s_gdTexture = new GLRenderCtx({ winSize.size.width, winSize.size.height });
        }

        if (!s_gdTexture->begin()) {
            delete s_gdTexture;
            s_gdTexture = nullptr;
            CCDirector::drawScene();
            return;
        }
        CCDirector::drawScene();

        // s_gdTexture->end() is called in the preDrawCall callback of ImGuiCocos
    }
};

// For the one eclipse shortcut
struct ToggleDevToolsEvent : Event<ToggleDevToolsEvent, bool()> {
    using Event::Event;
};

$execute {
    ToggleDevToolsEvent().listen([] {
        DevTools::get()->toggle();
        return ListenerResult::Stop;
    }).leak();

    ImGuiCocos::get().setup([] {
        DevTools::get()->setup();
    }).draw([] {
        DevTools::get()->draw(s_gdTexture);
        DevTools::get()->skipMousePosHook() = false;
    }).setPreDrawCall([] {
        if (s_gdTexture)
            s_gdTexture->end();
        DevTools::get()->skipMousePosHook() = true;
    });
}
