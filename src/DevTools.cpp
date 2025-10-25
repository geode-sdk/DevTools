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
#include "nodes/DragButton.hpp"

template<>
struct matjson::Serialize<Settings> {
    static Result<Settings> fromJson(const matjson::Value& value) {
        Settings s;

        const auto assign = [](const matjson::Value& json, auto& prop) {
            if (auto res = json.as<std::decay_t<decltype(prop)>>()) prop = res.unwrap();
        };

        assign(value["game_in_window"], s.GDInWindow);
        assign(value["attributes_in_tree"], s.attributesInTree);
        assign(value["always_highlight"], s.alwaysHighlight);
        assign(value["highlight_layouts"], s.highlightLayouts);
        assign(value["arrow_expand"], s.arrowExpand);
        assign(value["order_children"], s.orderChildren);
        assign(value["advanced_settings"], s.advancedSettings);
        assign(value["show_memory_viewer"], s.showMemoryViewer);
        assign(value["show_mod_graph"], s.showModGraph);
        assign(value["theme"], s.theme);
        assign(value["theme_color"], s.themeColor);
        assign(value["button_x"], s.buttonPos.x);
        assign(value["button_y"], s.buttonPos.y);
        assign(value["button_editor"], s.buttonInEditor);
        assign(value["button_game"], s.buttonInGame);

        return Ok(s);
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
            { "button_x", settings.buttonPos.x },
            { "button_y", settings.buttonPos.y },
            { "button_editor", settings.buttonInEditor },
            { "button_game", settings.buttonInGame },
        });
    }
};

$on_mod(DataSaved) { DevTools::get()->saveSettings(); }

DevTools* DevTools::get() {
    static auto inst = new DevTools();
    return inst;
}

void DevTools::loadSettings() { m_settings = Mod::get()->getSavedValue<Settings>("settings"); }
void DevTools::saveSettings() {
    m_settings.buttonPos = DragButton::get()->getPosition();
    Mod::get()->setSavedValue("settings", m_settings);
}
Settings DevTools::getSettings() { return m_settings; }

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

void DevTools::addCustomCallback(std::function<void(CCNode*)> callback) {
    m_customCallbacks.push_back(std::move(callback));
}

void DevTools::drawPage(const char* name, void(DevTools::*pageFun)()) {
    if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {

        // Fix wrapping after window resize
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

        (this->*pageFun)();

        // Scroll when dragging (useful for android users)
        auto mouse_dt = ImGui::GetIO().MouseDelta;
        ImVec2 delta = ImGui::GetIO().MouseDownDuration[0] > 0.1 ? ImVec2(mouse_dt.x * -1, mouse_dt.y * -1) : ImVec2(0, 0);
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiWindow* window = g.CurrentWindow;
        if (!window) return;
        bool hovered = false;
        bool held = false;
        ImGuiID id = window->GetID("##scrolldraggingoverlay");
        ImGui::KeepAliveID(id);
        ImGuiButtonFlags button_flags = ImGuiButtonFlags_MouseButtonLeft;
        if (g.HoveredId == 0) // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
            ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
        if (held && fabs(delta.x) >= 0.1f) ImGui::SetScrollX(window, window->Scroll.x + delta.x);
        if (held && fabs(delta.y) >= 0.1f) ImGui::SetScrollY(window, window->Scroll.y + delta.y);

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

#ifdef GEODE_IS_WINDOWS // Windows exclusive cursor updates from imgui-cocos

    // Shows imgui's cursor instead of hidden cursor if out of GD Window
    auto isCursorVisible = false;
    CURSORINFO ci = { sizeof(ci) }; //winapi
    if (GetCursorInfo(&ci)) isCursorVisible = (ci.flags & CURSOR_SHOWING) != 0;
    ImGui::GetIO().MouseDrawCursor = m_visible and !isCursorVisible and !shouldPassEventsToGDButTransformed();

    struct GLFWCursorData {
        void* next = nullptr;
        HCURSOR cursor;
    };
    auto& cursorField = *reinterpret_cast<GLFWCursorData**>(reinterpret_cast<uintptr_t>(
        CCEGLView::get()->getWindow()) + 0x50);

    auto cursor = ImGui::GetIO().MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    static ImGuiMouseCursor lastCursor = ImGuiMouseCursor_COUNT;
    if (cursor != lastCursor) {
        lastCursor = cursor;
        auto winCursor = IDC_ARROW;
        switch (cursor)
        {
        case ImGuiMouseCursor_Arrow: winCursor = IDC_ARROW; break;
        case ImGuiMouseCursor_TextInput: winCursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll: winCursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW: winCursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS: winCursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW: winCursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE: winCursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand: winCursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed: winCursor = IDC_NO; break;
        }
        if (cursorField) {
            cursorField->cursor = LoadCursor(NULL, winCursor);
        }
        else {
            // must be heap allocated
            cursorField = new GLFWCursorData{
                .next = nullptr,
                .cursor = LoadCursor(NULL, winCursor)
            };
        }
    }
#endif

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

    // Allow user scaling text of individual window with CTRL+Wheel.
    io.FontAllowUserScaling = true;

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

bool DevTools::isVisible() {
    return m_visible;
}

void DevTools::sceneChanged() {
    m_selectedNode = nullptr;
}

bool DevTools::shouldUseGDWindow() const {
    return Mod::get()->getSettingValue<bool>("should-use-gd-window");
}