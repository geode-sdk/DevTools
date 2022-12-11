#pragma once

#include <imgui.h>
#include <iostream>
#include <cocos2d.h>

using namespace cocos2d;

static std::ostream& operator<<(std::ostream& stream, ImVec2 const& vec) {
    return stream << vec.x << ", " << vec.y;
}

static std::ostream& operator<<(std::ostream& stream, ImVec4 const& vec) {
    return stream << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w;
}

static ImVec2 operator+(ImVec2 const& a, ImVec2 const& b) {
    return {
        a.x + b.x,
        a.y + b.y
    };
}

static ImVec2 operator-(ImVec2 const& a, ImVec2 const& b) {
    return {
        a.x - b.x,
        a.y - b.y
    };
}

static ImVec2 operator-(ImVec2 const& a) {
    return { -a.x, -a.y };
}

static ImVec2 operator/(ImVec2 const& a, ImVec2 const& b) {
    return {
        a.x / b.x,
        a.y / b.y
    };
}

static ImVec2 operator/(ImVec2 const& a, int b) {
    return {
        a.x / b,
        a.y / b
    };
}

static ImVec2 operator/(ImVec2 const& a, float b) {
    return {
        a.x / b,
        a.y / b
    };
}

static bool operator!=(ImVec2 const& a, ImVec2 const& b) {
    return a.x != b.x || a.y != b.y;
}

static ImVec2 operator*(ImVec2 const& v1, float multi) {
    return { v1.x * multi, v1.y * multi };
}

static ImVec2 toVec2(CCPoint const& a) {
    const auto size = ImGui::GetMainViewport()->Size;
    const auto winSize = CCDirector::get()->getWinSize();
    return {
        a.x / winSize.width * size.x,
        (1.f - a.y / winSize.height) * size.y
    };
}

static ImVec2 toVec2(CCSize const& a) {
    const auto size = ImGui::GetMainViewport()->Size;
    const auto winSize = CCDirector::get()->getWinSize();
    return {
        a.width / winSize.width * size.x,
        -a.height / winSize.height * size.y
    };
}

static CCPoint toCocos(const ImVec2& pos) {
    const auto winSize = CCDirector::sharedDirector()->getWinSize();
    const auto size = ImGui::GetMainViewport()->Size;
    return CCPoint(
        pos.x / size.x * winSize.width,
        (1.f - pos.y / size.y) * winSize.height
    );
};