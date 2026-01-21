
#include "Geode/ui/SceneManager.hpp"
#include "platform/platform.hpp"
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/AchievementNotifier.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCNode.hpp>
#include <Geode/modify/GameToolbox.hpp>
#include "DevTools.hpp"
#include <imgui.h>
#include "ImGui.hpp"
#include "nodes/DragButton.hpp"

using namespace geode::prelude;

class $modify(CCNode) {
    void sortAllChildren() override {
        if (DevTools::get()->shouldOrderChildren()) {
            CCNode::sortAllChildren();
        }
    }
};

// todo: use shortcuts api once Geode has those
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool arr, double a4) {
        if (down && (key == KEY_F11 GEODE_MACOS(|| key == KEY_F10))) {
            DevTools::get()->toggle();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr, a4);
    }
};

#include <Geode/loader/GameEvent.hpp>
$execute {
    new EventListener<GameEventFilter>(+[](GameEvent*) {
        if (DevTools::get()->isButtonEnabled()) DevTools::get()->setupDragButton();
    }, GameEventFilter(GameEventType::Loaded));
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


class $modify(CCDirector) {
    void willSwitchToScene(CCScene* scene) {
        CCDirector::willSwitchToScene(scene);
        DevTools::get()->sceneChanged();
    }

    void drawScene() {
        if (!DevTools::get()->shouldUseGDWindow()) {
            return CCDirector::drawScene();
        }
        
        DevTools::get()->setup();

        static GLRenderCtx* gdTexture = nullptr;

        if (!DevTools::get()->shouldPopGame()) {
            if (gdTexture) {
                delete gdTexture;
                gdTexture = nullptr;
            }
            shouldPassEventsToGDButTransformed() = false;
            CCDirector::drawScene();
            return;
        }

        if (shouldUpdateGDRenderBuffer()) {
            if (gdTexture) {
                delete gdTexture;
                gdTexture = nullptr;
            }
            shouldUpdateGDRenderBuffer() = false;
        }

        auto winSize = this->getOpenGLView()->getViewPortRect() * geode::utils::getDisplayFactor();
        if (!gdTexture) {
            gdTexture = new GLRenderCtx({ winSize.size.width, winSize.size.height });
        }

        if (!gdTexture->begin()) {
            delete gdTexture;
            gdTexture = nullptr;
            CCDirector::drawScene();
            DevTools::get()->render(nullptr);
            return;
        }
        CCDirector::drawScene();
        gdTexture->end();

        DevTools::get()->render(gdTexture);

        // if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     auto backup_current_context = this->getOpenGLView()->getWindow();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     glfwMakeContextCurrent(backup_current_context);
        // }
    }
};

class $modify(CCEGLView) {
    // this is needed for popout mode because we need to render after gd has rendered,
    // but before the buffers have been swapped, which is not possible with just a
    // CCDirector::drawScene hook.
    void swapBuffers() {
        if (!DevTools::get()->shouldUseGDWindow() || !DevTools::get()->shouldPopGame()) {
            DevTools::get()->setup();
            DevTools::get()->render(nullptr);
        }
        CCEGLView::swapBuffers();
    }
};

// For the one eclipse shortcut
struct ToggleDevToolsEvent : geode::Event {
    ToggleDevToolsEvent() {}
};

$on_mod(Loaded) {
    new EventListener<EventFilter<ToggleDevToolsEvent>>(+[](ToggleDevToolsEvent* e) {
        DevTools::get()->toggle();
        return ListenerResult::Stop;
    });
}
