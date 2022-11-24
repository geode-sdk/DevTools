#pragma once

#include <imgui.h>
#include <string>
#include <vector>

struct ThemeDef {
    ImVec4 text;
    ImVec4 textDisabled;
    ImVec4 textSelectedBG;
    ImVec4 windowBG;
    ImVec4 childBG;
    ImVec4 popupBG;
    ImVec4 border;
    ImVec4 borderShadow;
    ImVec4 frameBG;
    ImVec4 frameBGHovered;
    ImVec4 frameBGActive;
    ImVec4 titleBarBG;
    ImVec4 titleBarBGCollapsed;
    ImVec4 titleBarBGActive;
    ImVec4 menuBarBG;
    ImVec4 scrollbarBG;
    ImVec4 scrollbarGrab;
    ImVec4 scrollbarGrabHovered;
    ImVec4 scrollbarGrabActive;
    ImVec4 checkMark;
    ImVec4 sliderGrab;
    ImVec4 sliderGrabActive;
    ImVec4 button;
    ImVec4 buttonHovered;
    ImVec4 buttonActive;
    ImVec4 header;
    ImVec4 headerHovered;
    ImVec4 headerActive;
    ImVec4 separator;
    ImVec4 separatorHovered;
    ImVec4 separatorActive;
    ImVec4 resizeGrip;
    ImVec4 resizeGripHovered;
    ImVec4 resizeGripActive;
    ImVec4 plotLines;
    ImVec4 plotLinesHovered;
    ImVec4 plotHistogram;
    ImVec4 plotHistogramHovered;
    ImVec4 dragDropTarget;
    ImVec4 navHighlight;
    ImVec4 navWindowingHighlight;
    ImVec4 tab;
    ImVec4 tabHovered;
    ImVec4 tabActive;
    ImVec4 tabUnfocused;
    ImVec4 tabUnfocusedActive;
    ImVec4 tableBorderStrong;
    ImVec4 tableBorderLight;
};

constexpr const char* DARK_THEME = "Dark";
constexpr const char* MATERIAL_DARK_THEME = "Material Dark";
constexpr const char* LIGHT_THEME = "Light";

ThemeDef& getThemeDef(std::string const& theme);
void applyTheme(std::string const& theme);

std::vector<std::string> getThemeOptions();
size_t getThemeIndex(std::string const& theme);
std::string getThemeAtIndex(size_t index);
