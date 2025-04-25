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

template<>
struct matjson::Serialize<Settings> {
    static Result<Settings> fromJson(const matjson::Value& value) {
        Settings defaults;

        return Ok(Settings {
            .GDInWindow = value["game_in_window"].asBool().unwrapOr(std::move(defaults.GDInWindow)),
            .attributesInTree = value["attributes_in_tree"].asBool().unwrapOr(std::move(defaults.attributesInTree)),
            .alwaysHighlight = value["always_highlight"].asBool().unwrapOr(std::move(defaults.alwaysHighlight)),
            .highlightLayouts = value["highlight_layouts"].asBool().unwrapOr(std::move(defaults.highlightLayouts)),
            .arrowExpand = value["arrow_expand"].asBool().unwrapOr(std::move(defaults.arrowExpand)),
            .orderChildren = value["order_children"].asBool().unwrapOr(std::move(defaults.orderChildren)),
            .advancedSettings = value["advanced_settings"].asBool().unwrapOr(std::move(defaults.advancedSettings)),
            .showMemoryViewer = value["show_memory_viewer"].asBool().unwrapOr(std::move(defaults.showMemoryViewer)),
            .showModGraph = value["show_mod_graph"].asBool().unwrapOr(std::move(defaults.showModGraph)),
            .theme = value["theme"].asString().unwrapOr(std::move(defaults.theme)),
            .themeColor = value["theme_color"].as<ccColor4B>().isOk() ? value["theme_color"].as<ccColor4B>().unwrap() : std::move(defaults.themeColor),
            .buttonScale = value["button_scale"].as<float>().unwrapOr(std::move(defaults.buttonScale)),
            .buttonOpacity = value["button_opacity"].as<int>().unwrapOr(std::move(defaults.buttonOpacity)),
            .buttonInGameplay = value["button_gameplay"].asBool().unwrapOr(std::move(defaults.buttonInGameplay)),
            .buttonInEditor = value["button_editor"].asBool().unwrapOr(std::move(defaults.buttonInEditor)),
        });
    }

    static matjson::Value toJson(const Settings& settings) {
        return matjson::makeObject({
            { "game_in_window", settings.GDInWindow },
            { "attributes_in_tree", settings.attributesInTree },
            { "always_highlight", settings.alwaysHighlight },
            { "highlight_layouts", settings.highlightLayouts },
            { "arrow_expand", settings.arrowExpand },
            { "order_children", settings.orderChildren },
            { "advanced_settings", settings.advancedSettings },
            { "show_memory_viewer", settings.showMemoryViewer },
            { "show_mod_graph", settings.showModGraph },
            { "theme", settings.theme },
            { "theme_color", settings.themeColor },
            { "button_scale", settings.buttonScale },
            { "button_opacity", settings.buttonOpacity },
            { "button_gameplay", settings.buttonInGameplay },
            { "button_editor", settings.buttonInEditor },
        });
    }
};

$on_mod(DataSaved) { DevTools::get()->saveSettings(); }

DevTools* DevTools::get() {
    static auto inst = new DevTools();
    return inst;
}

void DevTools::loadSettings() { 
    m_settings = Mod::get()->getSavedValue<Settings>("settings");
}
void DevTools::saveSettings() { Mod::get()->setSavedValue("settings", m_settings); }
Settings DevTools::getSettings() { return m_settings; }

Settings DevTools::getSettings() {
    return m_settings;
}

bool DevTools::shouldPopGame() const {
    return m_visible && m_settings.GDInWindow;
}

bool DevTools::pausedGame() const {
    return m_pauseGame;
}

bool DevTools::isSetup() const {
    return m_setup;
}

bool DevTools::shouldOrderChildren() const {
    return m_settings.orderChildren;
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

    if ((!Mod::get()->setSavedValue("layout-loaded", true) || m_shouldRelayout)) {
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

    // if advanced ever has more than one option, add it back
#if 0
    if (m_settings.advancedSettings) {
        this->drawPage(
                U8STR(FEATHER_SETTINGS " Advanced Settings###devtools/advanced/settings"),
                &DevTools::drawAdvancedSettings
        );
    }
#endif

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

    if (m_settings.showModGraph) {
        this->drawPage(
            U8STR(FEATHER_SHARE_2 " Mod Graph###devtools/advanced/mod-graph"),
            &DevTools::drawModGraph
        );
    }

    if (m_settings.showMemoryViewer) {
        this->drawPage(
            U8STR(FEATHER_TERMINAL " Memory viewer"), 
            &DevTools::drawMemory
        );
    }
}

void DevTools::draw(GLRenderCtx* ctx) {
    if (m_visible) {
        if (m_reloadTheme) {
            applyTheme(m_settings.theme);
            m_reloadTheme = false;
        }

        m_dockspaceID = ImGui::DockSpaceOverViewport(
            0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode
        );

        ImGui::PushFont(m_defaultFont);
        this->drawPages();
        if (m_selectedNode) {
            this->highlightNode(m_selectedNode, HighlightMode::Selected);
        }
        if (this->shouldUseGDWindow()) this->drawGD(ctx);
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
        // AddFontFromMemoryTTF assumes ownership of the passed data unless you configure it not to.
        // Our font data has static lifetime, so we're handling the ownership.

        ImFontConfig config;
        config.FontDataOwnedByAtlas = false;
        auto* result = io.Fonts->AddFontFromMemoryTTF(
            font, realSize, size, &config, range
        );
        config.MergeMode = true;
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

    ImGui::CreateContext();

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

void DevTools::destroy() {
    if (!m_setup) return;
    this->show(false);
    auto& io = ImGui::GetIO();
    io.BackendPlatformUserData = nullptr;
    m_fontTexture->release();
    m_fontTexture = nullptr;

    ImGui::DestroyContext();
    m_setup = false;
    m_reloadTheme = true;
}

void DevTools::show(bool visible) {
    m_visible = visible;

    auto& io = ImGui::GetIO();
    io.WantCaptureMouse = visible;
    io.WantCaptureKeyboard = visible;
}

void DevTools::toggle() {
    this->show(!m_visible);
}

void DevTools::sceneChanged() {
    m_selectedNode = nullptr;
}

bool DevTools::shouldUseGDWindow() const {
    return Mod::get()->getSettingValue<bool>("should-use-gd-window");
}