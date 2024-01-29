
#include <imgui_internal.h>
#include "DevTools.hpp"
#include "fonts/FeatherIcons.hpp"
#include "fonts/OpenSans.hpp"
#include "fonts/GeodeIcons.hpp"
#include "fonts/RobotoMono.hpp"
#include "fonts/SourceCodeProLight.hpp"
#include "platform/platform.hpp"
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include "ImGui.hpp"

DevTools* DevTools::get() {
    static auto inst = new DevTools;
    return inst;
}

bool DevTools::shouldPopGame() const {
    return m_visible && m_GDInWindow;
}

bool DevTools::pausedGame() const {
    return m_pauseGame;
}

bool DevTools::isSetup() const {
    return m_setup;
}

bool DevTools::shouldOrderChildren() const {
    return m_orderChildren;
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
    if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        (this->*pageFun)();
    }
    ImGui::End();
}

void DevTools::drawPages() {
    const auto size = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize();

    if (!Mod::get()->setSavedValue("layout-loaded", true) || m_shouldRelayout) {
        m_shouldRelayout = false;

        auto id = m_dockspaceID;
        ImGui::DockBuilderRemoveNode(id);
        ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_PassthruCentralNode);

        auto leftDock = ImGui::DockBuilderSplitNode(m_dockspaceID, ImGuiDir_Left, 0.3f, nullptr, &id);

        auto topLeftDock = ImGui::DockBuilderSplitNode(leftDock, ImGuiDir_Up, 0.4f, nullptr, &leftDock);

        auto bottomLeftTopHalfDock = ImGui::DockBuilderSplitNode(leftDock, ImGuiDir_Up, 0.6f, nullptr, &leftDock);

        ImGui::DockBuilderDockWindow("###devtools/tree", topLeftDock);
        ImGui::DockBuilderDockWindow("###devtools/settings", topLeftDock);
        ImGui::DockBuilderDockWindow("###devtools/advanced/settings", topLeftDock);
        ImGui::DockBuilderDockWindow("###devtools/attributes", bottomLeftTopHalfDock);
        ImGui::DockBuilderDockWindow("###devtools/preview", leftDock);
        ImGui::DockBuilderDockWindow("###devtools/geometry-dash", id);
        ImGui::DockBuilderDockWindow("###devtools/advanced/mod-graph", topLeftDock);
        ImGui::DockBuilderDockWindow("###devtools/advanced/mod-index", topLeftDock);

        ImGui::DockBuilderFinish(id);
    }

    this->drawPage(
        U8STR(FEATHER_GIT_MERGE " Tree###devtools/tree"),
        &DevTools::drawTree
    );

    this->drawPage(
        U8STR(FEATHER_SETTINGS " Settings###devtools/settings"),
        &DevTools::drawSettings
    );

    if (m_advancedSettings) {
        this->drawPage(
                U8STR(FEATHER_SETTINGS " Advanced Settings###devtools/advanced/settings"),
                &DevTools::drawAdvancedSettings
        );
    }

    this->drawPage(
        U8STR(FEATHER_TOOL " Attributes###devtools/attributes"),
        &DevTools::drawAttributes
    );

    // TODO: fix preview tab
#if 0
    this->drawPage(
        U8STR(FEATHER_DATABASE " Preview###devtools/preview"),
        &DevTools::drawPreview
    );
#endif

    if (m_showModGraph) {
        this->drawPage(
            U8STR(FEATHER_SHARE_2 " Mod Graph###devtools/advanced/mod-graph"),
            &DevTools::drawModGraph
        );
    }

    if (m_showModIndex) {
        this->drawPage(
            U8STR(FEATHER_LIST " Mod Index###devtools/advanced/mod-index"),
            &DevTools::drawModIndex
        );
    }

    this->drawPage("Memory viewer", &DevTools::drawMemory);
}

void DevTools::draw(GLRenderCtx* ctx) {
    if (m_visible) {
        if (m_reloadTheme) {
            applyTheme(m_theme);
            m_reloadTheme = false;
        }

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
    
    static constexpr auto add_font = [](
        void* font, size_t realSize, float size, const ImWchar* range
    ) {
        auto& io = ImGui::GetIO();
        ImFontConfig config;
        config.MergeMode = true;
        auto* result = io.Fonts->AddFontFromMemoryTTF(
            font, realSize, size, nullptr, range
        );
        io.Fonts->AddFontFromMemoryTTF(
            Font_FeatherIcons, sizeof(Font_FeatherIcons), size - 4.f, &config, icon_ranges
        );
        io.Fonts->Build();
        return result;
    };

    m_defaultFont = add_font(Font_OpenSans, sizeof(Font_OpenSans), 18.f, def_ranges);
    m_smallFont = add_font(Font_OpenSans, sizeof(Font_OpenSans), 10.f, def_ranges);
    m_monoFont = add_font(Font_RobotoMono, sizeof(Font_RobotoMono), 18.f, def_ranges);
    m_boxFont = add_font(Font_SourceCodeProLight, sizeof(Font_SourceCodeProLight), 23.f, box_ranges);
}

void DevTools::setup() {
    if (m_setup) return;
    m_setup = true;

    IMGUI_CHECKVERSION();
    
    auto ctx = ImGui::CreateContext();
    
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // if this is true then it just doesnt work :( why
    io.ConfigDockingWithShift = false;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsResizeFromEdges = true;
        
    this->setupFonts();
    this->setupPlatform();

#ifdef GEODE_IS_MOBILE
    ImGui::GetIO().FontGlobalScale = 2.f;
    ImGui::GetStyle().ScrollbarSize = 60.f;
    // ImGui::GetStyle().TabBarBorderSize = 60.f;
#endif
}

void DevTools::show(bool visible) {
    m_visible = visible;
}

void DevTools::toggle() {
    this->show(!m_visible);
    if (!m_visible) {
        ImGui::GetIO().WantCaptureMouse = false;
        ImGui::GetIO().WantCaptureKeyboard = false;
    }
}

void DevTools::sceneChanged() {
    m_selectedNode = nullptr;
}
