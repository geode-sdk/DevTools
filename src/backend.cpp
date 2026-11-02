#include <cocos2d.h>
#include <Geode/modify/CCTouchDispatcher.hpp>
#ifndef GEODE_IS_IOS
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#endif
#include <Geode/modify/CCIMEDispatcher.hpp>
#include "Geode/platform/cplatform.h"
#include "platform/platform.hpp"
#include "DevTools.hpp"
#include "ImGui.hpp"

using namespace cocos2d;

// based off https://github.com/matcool/gd-imgui-cocos

static bool g_useNormalPos = false;

#ifndef GEODE_IS_MOBILE

CCPoint getMousePos_H() {
    CCPoint mouse = cocos::getMousePos();
    const auto pos = toVec2(mouse);

    if (DevTools::get()->shouldUseGDWindow() && shouldPassEventsToGDButTransformed() && !g_useNormalPos) {
        auto win = ImGui::GetMainViewport()->Size;
        const auto gdRect = getGDWindowRect();

        auto relativePos = ImVec2(
            pos.x - gdRect.Min.x,
            pos.y - gdRect.Min.y
        );
        auto x = (relativePos.x / gdRect.GetWidth()) * win.x;
        auto y = (relativePos.y / gdRect.GetHeight()) * win.y;

        mouse = toCocos(ImVec2(x, y));
    }

    return mouse;
}

$execute {
    (void) Mod::get()->hook(
        reinterpret_cast<void*>(addresser::getNonVirtual(&geode::cocos::getMousePos)),
        &getMousePos_H,
        "geode::cocos::getMousePos"
    );
}

#endif

class $modify(CCTouchDispatcher) {
    static void onModify(auto& self) {
        /*
         * some mods hook this instead of using normal touch delegates for some reason
         * for example QOLMod, even in the rewrite
         * so i added hook priority
         */
        Result<> res = self.setHookPriorityPre("cocos2d::CCTouchDispatcher::touches", Priority::First);
        if (!res) {
            geode::log::warn("Failed to set hook priority for CCTouchDispatcher::touches: {}", res.unwrapErr());
        }
    }

    void touches(CCSet* touches, CCEvent* event, unsigned int type) {
        auto& io = ImGui::GetIO();
        auto* touch = static_cast<CCTouch*>(touches->anyObject());

        // for some reason mac can filter out out of touches i think?
        if (touch == nullptr) {
            return CCTouchDispatcher::touches(touches, event, type);
        }

        const auto pos = toVec2(touch->getLocation());
        GEODE_MOBILE(io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);)
        io.AddMousePosEvent(pos.x, pos.y);

        if (DevTools::get()->shouldUseGDWindow()) {
            auto win = ImGui::GetMainViewport()->Size;
            const auto gdRect = getGDWindowRect();
            if ((gdRect.Contains(pos) || touchFromGD()) && !DevTools::get()->pausedGame()) {
                auto relativePos = ImVec2(
                    pos.x - gdRect.Min.x,
                    pos.y - gdRect.Min.y
                );
                auto x = (relativePos.x / gdRect.GetWidth()) * win.x;
                auto y = (1.f - relativePos.y / gdRect.GetHeight()) * win.y;

                auto pos = toCocos(ImVec2(x, y));

                if (type == CCTOUCHBEGAN && shouldPassEventsToGDButTransformed()) {
                    touchFromGD() = true;
                    // makes the start location in the touch correct
                    touch->m_startPoint = pos;
                }

                if (touchFromGD()) {
                    // setTouchInfo messes up the previous location (causes issues like texturer loader's draggable nodes breaking)
                    touch->m_point = pos;
                    CCTouchDispatcher::touches(touches, event, type);

                    ImGui::SetWindowFocus(getTitle().c_str());
                    io.AddMouseButtonEvent(0, false);
                }
            }
        }
        if (io.WantCaptureMouse) {
            if (!touchFromGD()) {
                if (type == CCTOUCHBEGAN || type == CCTOUCHMOVED) {
                    GEODE_MOBILE(io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);)
                    io.AddMouseButtonEvent(0, true);
                }
                else {
                    io.AddMouseButtonEvent(0, false);
                }
            }

            if (type == CCTOUCHENDED || type == CCTOUCHCANCELLED) {
                touchFromGD() = false;
            }
        }
        else {
            if (type != CCTOUCHMOVED) {
                io.AddMouseButtonEvent(0, false);
            }
            if (!DevTools::get()->shouldUseGDWindow() || !DevTools::get()->shouldPopGame()) {
                touchFromGD() = false;
                CCTouchDispatcher::touches(touches, event, type);
            }
        }
    }
};
