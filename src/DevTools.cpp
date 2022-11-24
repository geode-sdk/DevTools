
#include <imgui_internal.h>
#include "DevTools.hpp"
#include "fonts/FeatherIcons.hpp"
#include "fonts/OpenSans.hpp"
#include "fonts/GeodeIcons.hpp"
#include "fonts/RobotoMono.hpp"
#include "fonts/SourceCodeProLight.hpp"
#include "platform/platform.hpp"
#include <Geode/utils/operators.hpp>
#include <Geode/loader/Log.hpp>
#include "ImGui.hpp"

DevTools* DevTools::get() {
    static auto inst = new DevTools;
    return inst;
}

bool DevTools::shouldPopGame() const {
    return m_visible && m_GDInWindow;
}

CCNode* DevTools::getSelectedNode() const {
    return m_selectedNode;
}

void DevTools::selectNode(CCNode* node) {
    m_selectedNode = node;
}

void DevTools::highlightNode(CCNode* node, HighlightMode mode) {
    m_toHighlight.push_back({ node, mode });
}

void DevTools::drawPage(const char* name, void(DevTools::*pageFun)()) {
    if (ImGui::Begin(name)) {
        (this->*pageFun)();
    }
    ImGui::End();
}

void DevTools::drawPages() {
    this->drawPage(
        U8STR(FEATHER_GIT_MERGE " Tree"),
        &DevTools::drawTree
    );
    this->drawPage(
        U8STR(FEATHER_SETTINGS " Settings"),
        &DevTools::drawSettings
    );
    this->drawPage(
        U8STR(FEATHER_TOOL " Attributes"),
        &DevTools::drawAttributes
    );
    this->drawPage(
        U8STR(FEATHER_DATABASE " Layout"),
        &DevTools::drawLayout
    );
}

void DevTools::draw(GLRenderCtx* ctx) {
    if (m_visible) {
        if (m_reloadTheme) {
            applyTheme(m_theme);
            m_reloadTheme = false;
        }

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

        m_dockspaceID = ImGui::DockSpaceOverViewport(
            nullptr, ImGuiDockNodeFlags_PassthruCentralNode
        );

        ImGui::PushFont(m_defaultFont);
        this->drawPages();
        if (m_selectedNode) {
            this->highlightNode(m_selectedNode, HighlightMode::Selected);
        }
        this->drawGD(ctx);
        ImGui::PopFont();
    }
}

void DevTools::setupFonts() {
    static const ImWchar icon_ranges[] = { FEATHER_MIN_FA, FEATHER_MAX_FA, 0 };
    static const ImWchar box_ranges[]  = { BOX_DRAWING_MIN_FA, BOX_DRAWING_MAX_FA, 0 };
    static const ImWchar* def_ranges   = ImGui::GetIO().Fonts->GetGlyphRangesDefault();
    
    static const auto add_font = +[](
        ImFont** member, void* font, float size, const ImWchar* range
    ) -> void {
        auto& io = ImGui::GetIO();
        ImFontConfig config;
        config.MergeMode = true;
        *member = io.Fonts->AddFontFromMemoryTTF(
            font, sizeof font, size, nullptr, range
        );
        io.Fonts->AddFontFromMemoryTTF(
            Font_FeatherIcons, sizeof Font_FeatherIcons, size - 4.f, &config, icon_ranges
        );
        io.Fonts->Build();
    };

    add_font(&m_defaultFont, Font_OpenSans,           18.f, def_ranges);
    add_font(&m_smallFont,   Font_OpenSans,           10.f, def_ranges);
    add_font(&m_monoFont,    Font_RobotoMono,         18.f, def_ranges);
    add_font(&m_boxFont,     Font_SourceCodeProLight, 23.f, box_ranges);
}

void DevTools::setup() {
    if (m_setup) return;
    m_setup = true;

    IMGUI_CHECKVERSION();
    
    ImGui::CreateContext();
    
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = true;

    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    this->setupFonts();
    this->setupPlatform();
}

void DevTools::show(bool visible) {
    m_visible = visible;
}

void DevTools::toggle() {
    this->show(!m_visible);
}

void DevTools::sceneChanged() {
    m_selectedNode = nullptr;
}
