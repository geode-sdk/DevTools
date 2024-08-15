#include "themes.hpp"
#include <Geode/utils/ranges.hpp>
#include <Geode/utils/general.hpp>
#include <cocos2d.h>

using namespace geode::prelude;

namespace {
    constexpr float clamp(float v) {
        return v > 1.f ? 1.f : v < 0 ? 0 : v;
    }

    constexpr ImVec4 color(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
        return {
            r / 255.f,
            g / 255.f,
            b / 255.f,
            a / 255.f,
        };
    }

    constexpr ImVec4 color(GLubyte v) {
        return {
            v / 255.f,
            v / 255.f,
            v / 255.f,
            v / 255.f,
        };
    }

    constexpr ImVec4 operator+(ImVec4 const& vec, int value) {
        return {
            clamp(vec.x + value / 255.f),
            clamp(vec.y + value / 255.f),
            clamp(vec.z + value / 255.f),
            vec.w,
        };
    }

    constexpr ImVec4 operator-(ImVec4 const& vec, int value) {
        return {
            clamp(vec.x - value / 255.f),
            clamp(vec.y - value / 255.f),
            clamp(vec.z - value / 255.f),
            vec.w,
        };
    }

    struct Alpha {
        GLubyte value;
    };

    constexpr Alpha alpha(GLubyte v) {
        return { v };
    }

    constexpr ImVec4 operator+(ImVec4 const& vec, Alpha value) {
        return {
            vec.x,
            vec.y,
            vec.z,
            clamp(vec.w + value.value / 255.f),
        };
    }

    constexpr ImVec4 operator-(ImVec4 const& vec, Alpha value) {
        return {
            vec.x,
            vec.y,
            vec.z,
            clamp(vec.w - value.value / 255.f),
        };
    }
}

constexpr ImVec4 DARK_THEME_BG            = color(33, 33, 33, 255);
constexpr ImVec4 DARK_THEME_BG_DARK       = color(22, 22, 22, 255);
constexpr ImVec4 DARK_THEME_TEXT          = color(255, 255, 255, 255);
constexpr ImVec4 DARK_THEME_LIGHT         = color(255, 255, 255, 255);
constexpr ImVec4 DARK_THEME_PRIMARY       = color(2, 119, 189, 255);
constexpr ImVec4 DARK_THEME_PRIMARY_DARK  = color(2, 66, 104, 255);
constexpr ImVec4 DARK_THEME_PRIMARY_LIGHT = color(73, 164, 216, 255);

ThemeDef DARK_THEME_DEF {
    .text                   = DARK_THEME_TEXT,
    .textDisabled           = DARK_THEME_TEXT - 102,
    .textSelectedBG         = DARK_THEME_PRIMARY - 60 - alpha(180),
    .windowBG               = DARK_THEME_BG,
    .childBG                = DARK_THEME_BG,
    .popupBG                = DARK_THEME_BG,
    .border                 = DARK_THEME_BG + 23,
    .borderShadow           = DARK_THEME_BG + 50,
    .frameBG                = DARK_THEME_BG_DARK,
    .frameBGHovered         = DARK_THEME_BG_DARK + 20,
    .frameBGActive          = DARK_THEME_BG_DARK,
    .titleBarBG             = DARK_THEME_PRIMARY_DARK,
    .titleBarBGCollapsed    = DARK_THEME_PRIMARY_DARK - alpha(150),
    .titleBarBGActive       = DARK_THEME_PRIMARY,
    .menuBarBG              = DARK_THEME_BG_DARK,
    .scrollbarBG            = DARK_THEME_BG_DARK,
    .scrollbarGrab          = DARK_THEME_PRIMARY,
    .scrollbarGrabHovered   = DARK_THEME_PRIMARY_LIGHT,
    .scrollbarGrabActive    = DARK_THEME_PRIMARY_DARK,
    .checkMark              = DARK_THEME_PRIMARY,
    .sliderGrab             = DARK_THEME_PRIMARY,
    .sliderGrabActive       = DARK_THEME_PRIMARY_DARK,
    .button                 = DARK_THEME_PRIMARY,
    .buttonHovered          = DARK_THEME_PRIMARY_LIGHT,
    .buttonActive           = DARK_THEME_PRIMARY_DARK,
    .header                 = DARK_THEME_PRIMARY - alpha(140),
    .headerHovered          = DARK_THEME_PRIMARY_LIGHT - alpha(40),
    .headerActive           = DARK_THEME_PRIMARY_DARK,
    .separator              = DARK_THEME_LIGHT - alpha(140),
    .separatorHovered       = DARK_THEME_LIGHT - alpha(40),
    .separatorActive        = DARK_THEME_PRIMARY_DARK,
    .resizeGrip             = DARK_THEME_LIGHT - alpha(200),
    .resizeGripHovered      = DARK_THEME_LIGHT - alpha(100),
    .resizeGripActive       = DARK_THEME_PRIMARY_DARK,
    .plotLines              = DARK_THEME_PRIMARY - alpha(140),
    .plotLinesHovered       = DARK_THEME_PRIMARY - alpha(40),
    .plotHistogram          = DARK_THEME_PRIMARY - alpha(140),
    .plotHistogramHovered   = DARK_THEME_PRIMARY - alpha(40),
    .dragDropTarget         = DARK_THEME_PRIMARY - 60 - alpha(20),
    .navHighlight           = DARK_THEME_PRIMARY - 30 - alpha(40),
    .navWindowingHighlight  = DARK_THEME_PRIMARY - 40 - alpha(40),
    .tab                    = DARK_THEME_PRIMARY - alpha(200),
    .tabHovered             = DARK_THEME_PRIMARY,
    .tabActive              = DARK_THEME_PRIMARY_LIGHT,
    .tabUnfocused           = DARK_THEME_PRIMARY_DARK - alpha(140),
    .tabUnfocusedActive     = DARK_THEME_PRIMARY - alpha(140),
    .tableBorderStrong      = DARK_THEME_LIGHT - alpha(50),
    .tableBorderLight       = DARK_THEME_LIGHT - alpha(100),
};

constexpr ImVec4 MATERIAL_DARK_THEME_BG            = color(33,  33,  33, 255);
constexpr ImVec4 MATERIAL_DARK_THEME_BG_DARK       = color(22,  22,  22, 255);
constexpr ImVec4 MATERIAL_DARK_THEME_BG_LIGHT      = color(45,  45,  45, 255);
constexpr ImVec4 MATERIAL_DARK_THEME_TEXT          = color(255, 255, 255, 255);
constexpr ImVec4 MATERIAL_DARK_THEME_LIGHT         = color(255, 255, 255, 255);
constexpr ImVec4 MATERIAL_DARK_THEME_PRIMARY       = color(70,  70,  70, 255);
constexpr ImVec4 MATERIAL_DARK_THEME_PRIMARY_DARK  = color(56,  56,  56, 255);
constexpr ImVec4 MATERIAL_DARK_THEME_PRIMARY_LIGHT = color(88,  88,  88, 255);

ThemeDef MATERIAL_DARK_THEME_DEF {
    .text                   = MATERIAL_DARK_THEME_TEXT,
    .textDisabled           = MATERIAL_DARK_THEME_TEXT - 102,
    .textSelectedBG         = MATERIAL_DARK_THEME_PRIMARY - 60 - alpha(180),
    .windowBG               = MATERIAL_DARK_THEME_BG,
    .childBG                = MATERIAL_DARK_THEME_BG,
    .popupBG                = MATERIAL_DARK_THEME_BG,
    .border                 = MATERIAL_DARK_THEME_BG + 23,
    .borderShadow           = MATERIAL_DARK_THEME_BG + 50,
    .frameBG                = MATERIAL_DARK_THEME_BG_DARK,
    .frameBGHovered         = MATERIAL_DARK_THEME_BG_DARK + 20,
    .frameBGActive          = MATERIAL_DARK_THEME_BG_DARK,
    .titleBarBG             = MATERIAL_DARK_THEME_BG_DARK,
    .titleBarBGCollapsed    = MATERIAL_DARK_THEME_BG_DARK - alpha(150),
    .titleBarBGActive       = MATERIAL_DARK_THEME_BG_DARK,
    .menuBarBG              = MATERIAL_DARK_THEME_BG_DARK,
    .scrollbarBG            = MATERIAL_DARK_THEME_BG_DARK,
    .scrollbarGrab          = MATERIAL_DARK_THEME_PRIMARY,
    .scrollbarGrabHovered   = MATERIAL_DARK_THEME_PRIMARY_LIGHT,
    .scrollbarGrabActive    = MATERIAL_DARK_THEME_PRIMARY_DARK,
    .checkMark              = MATERIAL_DARK_THEME_TEXT,
    .sliderGrab             = MATERIAL_DARK_THEME_PRIMARY,
    .sliderGrabActive       = MATERIAL_DARK_THEME_PRIMARY_DARK,
    .button                 = MATERIAL_DARK_THEME_PRIMARY,
    .buttonHovered          = MATERIAL_DARK_THEME_PRIMARY_LIGHT,
    .buttonActive           = MATERIAL_DARK_THEME_PRIMARY_DARK,
    .header                 = MATERIAL_DARK_THEME_PRIMARY - alpha(140),
    .headerHovered          = MATERIAL_DARK_THEME_PRIMARY_LIGHT - alpha(40),
    .headerActive           = MATERIAL_DARK_THEME_PRIMARY_DARK,
    .separator              = MATERIAL_DARK_THEME_LIGHT - alpha(140),
    .separatorHovered       = MATERIAL_DARK_THEME_LIGHT - alpha(40),
    .separatorActive        = MATERIAL_DARK_THEME_PRIMARY_DARK,
    .resizeGrip             = MATERIAL_DARK_THEME_LIGHT - alpha(200),
    .resizeGripHovered      = MATERIAL_DARK_THEME_LIGHT - alpha(100),
    .resizeGripActive       = MATERIAL_DARK_THEME_PRIMARY_DARK,
    .plotLines              = MATERIAL_DARK_THEME_PRIMARY - alpha(140),
    .plotLinesHovered       = MATERIAL_DARK_THEME_PRIMARY - alpha(40),
    .plotHistogram          = MATERIAL_DARK_THEME_PRIMARY - alpha(140),
    .plotHistogramHovered   = MATERIAL_DARK_THEME_PRIMARY - alpha(40),
    .dragDropTarget         = MATERIAL_DARK_THEME_PRIMARY - 60 - alpha(20),
    .navHighlight           = MATERIAL_DARK_THEME_PRIMARY - 30 - alpha(40),
    .navWindowingHighlight  = MATERIAL_DARK_THEME_PRIMARY - 40 - alpha(40),
    .tab                    = MATERIAL_DARK_THEME_BG_DARK,
    .tabHovered             = MATERIAL_DARK_THEME_BG_LIGHT,
    .tabActive              = MATERIAL_DARK_THEME_BG,
    .tabUnfocused           = MATERIAL_DARK_THEME_BG_DARK - alpha(140),
    .tabUnfocusedActive     = MATERIAL_DARK_THEME_BG - alpha(140),
    .tableBorderStrong      = MATERIAL_DARK_THEME_LIGHT - alpha(50),
    .tableBorderLight       = MATERIAL_DARK_THEME_LIGHT - alpha(100),
};

constexpr ImVec4 LIGHT_THEME_BG            = color(233, 233, 233, 255);
constexpr ImVec4 LIGHT_THEME_BG_DARK       = color(200, 200, 200, 255);
constexpr ImVec4 LIGHT_THEME_TEXT          = color(33,  33,  33, 255);
constexpr ImVec4 LIGHT_THEME_LIGHT         = color(200, 200, 200, 255);
constexpr ImVec4 LIGHT_THEME_PRIMARY       = color(160, 160, 160, 255);
constexpr ImVec4 LIGHT_THEME_PRIMARY_DARK  = color(140, 140, 140, 255);
constexpr ImVec4 LIGHT_THEME_PRIMARY_LIGHT = color(190, 190, 190, 255);

ThemeDef LIGHT_THEME_DEF {
    .text                   = LIGHT_THEME_TEXT,
    .textDisabled           = LIGHT_THEME_TEXT - 102,
    .textSelectedBG         = LIGHT_THEME_PRIMARY - 60 - alpha(180),
    .windowBG               = LIGHT_THEME_BG,
    .childBG                = LIGHT_THEME_BG,
    .popupBG                = LIGHT_THEME_BG,
    .border                 = LIGHT_THEME_BG + 23,
    .borderShadow           = LIGHT_THEME_BG + 50,
    .frameBG                = LIGHT_THEME_BG_DARK,
    .frameBGHovered         = LIGHT_THEME_BG_DARK + 20,
    .frameBGActive          = LIGHT_THEME_BG_DARK,
    .titleBarBG             = LIGHT_THEME_BG_DARK,
    .titleBarBGCollapsed    = LIGHT_THEME_BG_DARK - alpha(150),
    .titleBarBGActive       = LIGHT_THEME_BG,
    .menuBarBG              = LIGHT_THEME_BG_DARK,
    .scrollbarBG            = LIGHT_THEME_BG_DARK,
    .scrollbarGrab          = LIGHT_THEME_PRIMARY,
    .scrollbarGrabHovered   = LIGHT_THEME_PRIMARY_LIGHT,
    .scrollbarGrabActive    = LIGHT_THEME_PRIMARY_DARK,
    .checkMark              = LIGHT_THEME_TEXT,
    .sliderGrab             = LIGHT_THEME_PRIMARY,
    .sliderGrabActive       = LIGHT_THEME_PRIMARY_DARK,
    .button                 = LIGHT_THEME_PRIMARY,
    .buttonHovered          = LIGHT_THEME_PRIMARY_LIGHT,
    .buttonActive           = LIGHT_THEME_PRIMARY_DARK,
    .header                 = LIGHT_THEME_PRIMARY - alpha(140),
    .headerHovered          = LIGHT_THEME_PRIMARY_LIGHT - alpha(40),
    .headerActive           = LIGHT_THEME_PRIMARY_DARK,
    .separator              = LIGHT_THEME_LIGHT - alpha(140),
    .separatorHovered       = LIGHT_THEME_LIGHT - alpha(40),
    .separatorActive        = LIGHT_THEME_PRIMARY_DARK,
    .resizeGrip             = LIGHT_THEME_LIGHT - alpha(200),
    .resizeGripHovered      = LIGHT_THEME_LIGHT - alpha(100),
    .resizeGripActive       = LIGHT_THEME_PRIMARY_DARK,
    .plotLines              = LIGHT_THEME_PRIMARY - alpha(140),
    .plotLinesHovered       = LIGHT_THEME_PRIMARY - alpha(40),
    .plotHistogram          = LIGHT_THEME_PRIMARY - alpha(140),
    .plotHistogramHovered   = LIGHT_THEME_PRIMARY - alpha(40),
    .dragDropTarget         = LIGHT_THEME_PRIMARY - 60 - alpha(20),
    .navHighlight           = LIGHT_THEME_PRIMARY - 30 - alpha(40),
    .navWindowingHighlight  = LIGHT_THEME_PRIMARY - 40 - alpha(40),
    .tab                    = LIGHT_THEME_PRIMARY - alpha(200),
    .tabHovered             = LIGHT_THEME_PRIMARY,
    .tabActive              = LIGHT_THEME_PRIMARY_LIGHT,
    .tabUnfocused           = LIGHT_THEME_PRIMARY_DARK - alpha(140),
    .tabUnfocusedActive     = LIGHT_THEME_PRIMARY - alpha(140),
    .tableBorderStrong      = LIGHT_THEME_LIGHT - alpha(50),
    .tableBorderLight       = LIGHT_THEME_LIGHT - alpha(100),
};

static std::vector<std::string> THEME_OPTIONS = {
    DARK_THEME,
    MATERIAL_DARK_THEME,
    LIGHT_THEME,
};

void applyCommon(ImGuiStyle& style) {
    // style.WindowRounding    = 2.0f;
    // style.ScrollbarRounding = 3.0f;
    // style.GrabRounding      = 1.0f;
    // style.AntiAliasedLines  = true;
    // style.AntiAliasedFill   = true;
    // style.WindowRounding    = 1;
    // style.ChildRounding     = 2;
    // style.ScrollbarSize     = 16;
    // style.ScrollbarRounding = 3;
    // style.GrabRounding      = 2;
    // style.ItemSpacing.x     = 14;
    // style.ItemSpacing.y     = 8;
    // style.IndentSpacing     = 22;
    // style.FramePadding.x    = 8;
    // style.FramePadding.y    = 4;
    // style.Alpha             = 1.0f;
    // style.FrameRounding     = 2.0f;
    // style.WindowPadding     = { 3.f, 3.f };
    // style.ColorButtonPosition = ImGuiDir_Left;
}

ThemeDef& getThemeDef(std::string const& name) {
    switch (hash(name.c_str())) {
        case hash(DARK_THEME): return DARK_THEME_DEF;
        case hash(MATERIAL_DARK_THEME): return MATERIAL_DARK_THEME_DEF;
        case hash(LIGHT_THEME): return LIGHT_THEME_DEF;
    }
    return DARK_THEME_DEF;
}

void applyTheme(std::string const& name) {
    auto& style = ImGui::GetStyle();
    applyCommon(style);

    auto& theme = getThemeDef(name);
    style.Colors[ImGuiCol_Text] = theme.text;
    style.Colors[ImGuiCol_TextDisabled] = theme.textDisabled;
    style.Colors[ImGuiCol_TextSelectedBg] = theme.textSelectedBG;
    style.Colors[ImGuiCol_WindowBg] = theme.windowBG;
    style.Colors[ImGuiCol_ChildBg] = theme.childBG;
    style.Colors[ImGuiCol_PopupBg] = theme.popupBG;
    style.Colors[ImGuiCol_Border] = theme.border;
    style.Colors[ImGuiCol_BorderShadow] = theme.borderShadow;
    style.Colors[ImGuiCol_FrameBg] = theme.frameBG;
    style.Colors[ImGuiCol_FrameBgHovered] = theme.frameBGHovered;
    style.Colors[ImGuiCol_FrameBgActive] = theme.frameBGActive;
    style.Colors[ImGuiCol_TitleBg] = theme.titleBarBG;
    style.Colors[ImGuiCol_TitleBgCollapsed] = theme.titleBarBGCollapsed;
    style.Colors[ImGuiCol_TitleBgActive] = theme.titleBarBGActive;
    style.Colors[ImGuiCol_MenuBarBg] = theme.menuBarBG;
    style.Colors[ImGuiCol_ScrollbarBg] = theme.scrollbarBG;
    style.Colors[ImGuiCol_ScrollbarGrab] = theme.scrollbarGrab;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = theme.scrollbarGrabHovered;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = theme.scrollbarGrabActive;
    style.Colors[ImGuiCol_CheckMark] = theme.checkMark;
    style.Colors[ImGuiCol_SliderGrab] = theme.sliderGrab;
    style.Colors[ImGuiCol_SliderGrabActive] = theme.sliderGrabActive;
    style.Colors[ImGuiCol_Button] = theme.button;
    style.Colors[ImGuiCol_ButtonHovered] = theme.buttonHovered;
    style.Colors[ImGuiCol_ButtonActive] = theme.buttonActive;
    style.Colors[ImGuiCol_Header] = theme.header;
    style.Colors[ImGuiCol_HeaderHovered] = theme.headerHovered;
    style.Colors[ImGuiCol_HeaderActive] = theme.headerActive;
    style.Colors[ImGuiCol_Separator] = theme.separator;
    style.Colors[ImGuiCol_SeparatorHovered] = theme.separatorHovered;
    style.Colors[ImGuiCol_SeparatorActive] = theme.separatorActive;
    style.Colors[ImGuiCol_ResizeGrip] = theme.resizeGrip;
    style.Colors[ImGuiCol_ResizeGripHovered] = theme.resizeGripHovered;
    style.Colors[ImGuiCol_ResizeGripActive] = theme.resizeGripActive;
    style.Colors[ImGuiCol_PlotLines] = theme.plotLines;
    style.Colors[ImGuiCol_PlotLinesHovered] = theme.plotLinesHovered;
    style.Colors[ImGuiCol_PlotHistogram] = theme.plotHistogram;
    style.Colors[ImGuiCol_PlotHistogramHovered] = theme.plotHistogramHovered;
    style.Colors[ImGuiCol_DragDropTarget] = theme.dragDropTarget;
    style.Colors[ImGuiCol_NavHighlight] = theme.navHighlight;
    style.Colors[ImGuiCol_NavWindowingHighlight] = theme.navWindowingHighlight;
    style.Colors[ImGuiCol_Tab] = theme.tab;
    style.Colors[ImGuiCol_TabHovered] = theme.tabHovered;
    style.Colors[ImGuiCol_TabSelected] = theme.tabActive;
    style.Colors[ImGuiCol_TabDimmed] = theme.tabUnfocused;
    style.Colors[ImGuiCol_TabDimmedSelected] = theme.tabUnfocusedActive;
    style.Colors[ImGuiCol_TableBorderStrong] = theme.tableBorderStrong;
    style.Colors[ImGuiCol_TableBorderLight] = theme.tableBorderLight;
}

std::vector<std::string> getThemeOptions() {
    return THEME_OPTIONS;
}

size_t getThemeIndex(std::string const& theme) {
    return ranges::indexOf(THEME_OPTIONS, theme).value_or(0);
}

std::string getThemeAtIndex(size_t index) {
    return THEME_OPTIONS.at(index);
}
