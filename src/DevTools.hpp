#pragma once

#include "platform/platform.hpp"
#include <imgui.h>
#include "themes.hpp"
#include <cocos2d.h>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/ModMetadata.hpp>
#include <unordered_map>

using namespace geode::prelude;

enum class HighlightMode {
    Selected,
    Hovered,
    Layout,
};

class DevTools {
protected:
    bool m_visible = false;
    bool m_setup = false;
    bool m_reloadTheme = true;
    bool m_GDInWindow = true;
    bool m_attributesInTree = false;
    bool m_alwaysHighlight = true;
    bool m_shouldRelayout = false;
    bool m_highlightLayouts = false;
    bool m_arrowExpand = false;
    bool m_advancedSettings = false;
    bool m_showModGraph = false;
    bool m_pauseGame = false;
    bool m_orderChildren = true;
    bool m_showMemoryViewer = false;
    std::string m_theme = DARK_THEME;
    ImGuiID m_dockspaceID;
    ImFont* m_defaultFont  = nullptr;
    ImFont* m_smallFont    = nullptr;
    ImFont* m_monoFont     = nullptr;
    ImFont* m_boxFont      = nullptr;
    Ref<CCNode> m_selectedNode;
    std::vector<std::pair<CCNode*, HighlightMode>> m_toHighlight;

    void setupFonts();
    void setupPlatform();

    void drawTree();
    void drawTreeBranch(CCNode* node, size_t index);
    void drawSettings();
    void drawAdvancedSettings();
    void drawNodeAttributes(CCNode* node);
    void drawAttributes();
    void drawPreview();
    void drawNodePreview(CCNode* node);
    void drawHighlight(CCNode* node, HighlightMode mode);
    void drawLayoutHighlights(CCNode* node);
    void drawGD(GLRenderCtx* ctx);
    void drawModGraph();
    void drawModGraphNode(Mod* node);
    ModMetadata inputMetadata(void* treePtr, ModMetadata metadata);
    void drawPage(const char* name, void(DevTools::* fun)());
    void drawPages();
    void drawMemory();
    void draw(GLRenderCtx* ctx);

    void newFrame();
    void renderDrawData(ImDrawData*);
    void renderDrawDataFallback(ImDrawData*);

    bool hasExtension(const std::string& ext) const;

public:
    static DevTools* get();

    bool shouldUseGDWindow() const;

    bool shouldPopGame() const;
    bool pausedGame() const;
    bool isSetup() const;
    bool shouldOrderChildren() const;

    CCNode* getSelectedNode() const;
    void selectNode(CCNode* node);
    void highlightNode(CCNode* node, HighlightMode mode);

    void sceneChanged();

    void render(GLRenderCtx* ctx);

    // setup ImGui & DevTools
    void setup();

    void show(bool visible);
    void toggle();
};
