
#include "platform/platform.hpp"
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/AchievementNotifier.hpp>
#include <Geode/modify/CCDirector.hpp>
#include "DevTools.hpp"
#include <imgui.h>
#include "ImGui.hpp"

USE_GEODE_NAMESPACE();

// todo: use shortcuts api once Geode has those
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down) {
        if (down && key == KEY_F11) {
            DevTools::get()->toggle();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down);
    }
};

class $modify(AchievementNotifier) {
    void willSwitchToScene(CCScene* scene) {
        AchievementNotifier::willSwitchToScene(scene);
        DevTools::get()->sceneChanged();
    }
};

class $modify(CCDirector) {
    void drawScene() {
        DevTools::get()->setup();

        static GLRenderCtx* gdTexture = nullptr;

        if (!DevTools::get()->shouldPopGame()) {
            if (gdTexture) {
                delete gdTexture;
                gdTexture = nullptr;
            }
            shouldPassEventsToGDButTransformed() = false;
            CCDirector::drawScene();
            DevTools::get()->render(nullptr);
            return;
        }

        if (shouldUpdateGDRenderBuffer()) {
            if (gdTexture) {
                delete gdTexture;
                gdTexture = nullptr;
            }
            shouldUpdateGDRenderBuffer() = false;
        }

        auto winSize = this->getOpenGLView()->getViewPortRect();
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
