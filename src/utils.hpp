#pragma once

#include <cocos2d.h>
#include "platform/platform.hpp"
#include "DevTools.hpp"
#include "ImGui.hpp"

inline cocos2d::CCPoint convertTouch(cocos2d::CCPoint gdpos) {
    auto win = ImGui::GetMainViewport()->Size;
    const auto gdRect = getGDWindowRect();

    auto pos = toVec2(gdpos);
    if (gdRect.Contains(pos) && !DevTools::get()->pausedGame()) {
        auto relativePos = ImVec2(
            pos.x - gdRect.Min.x,
            pos.y - gdRect.Min.y
        );
        auto x = (relativePos.x / gdRect.GetWidth()) * win.x;
        auto y = relativePos.y / gdRect.GetHeight() * win.y;

        auto pos = toCocos(ImVec2(x, y));

        return pos;
    }

    return {};
}
