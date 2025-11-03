#pragma once

#include "platform/platform.hpp"
#include <imgui.h>
#include "themes.hpp"
#include <cocos2d.h>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/addresser.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/ModMetadata.hpp>

using namespace geode::prelude;

enum class HighlightMode {
    Selected,
    Hovered,
    Layout,
};

struct Settings {
    bool GDInWindow = true;
    bool attributesInTree = false;
    bool alwaysHighlight = true;
    bool highlightLayouts = false;
    bool arrowExpand = false;
    bool orderChildren = true;
    bool advancedSettings = false;
    bool showMemoryViewer = false;
    bool showModGraph = false;
    std::string theme = DARK_THEME;
    ccColor4B themeColor = {2, 119, 189, 255};
    CCPoint buttonPos = {50, 50};
    bool buttonInEditor = false;
    bool buttonInGame = false;
    bool enableMoving = false;
};

class DevTools {
protected:
    bool m_visible = false;
    bool m_setup = false;
    bool m_reloadTheme = true;
    bool m_shouldRelayout = false;
    bool m_showModGraph = false;
    bool m_pauseGame = false;
    Settings m_settings;
    ImGuiID m_dockspaceID;
    ImFont* m_defaultFont  = nullptr;
    ImFont* m_smallFont    = nullptr;
    ImFont* m_monoFont     = nullptr;
    ImFont* m_boxFont      = nullptr;
    CCTexture2D* m_fontTexture = nullptr;
    Ref<CCNode> m_selectedNode;
    Ref<CCNode> m_draggedNode;
    std::vector<std::pair<CCNode*, HighlightMode>> m_toHighlight;
    std::vector<std::function<void(CCNode*)>> m_customCallbacks;
    std::string m_searchQuery;
    std::string m_prevQuery;
    std::unordered_map<CCNode*, bool> m_nodeOpen;

    void setupFonts();
    void setupPlatform();

    void drawTree();
    void drawTreeBranch(CCNode* node, size_t index, bool drag);
    void drawSettings();
    void drawAdvancedSettings();
    void drawNodeAttributes(CCNode* node);
    void drawAttributes();
    void drawBasicAttributes(CCNode* node);
    void drawColorAttributes(CCNode* node);
    void drawLabelAttributes(CCNode* node);
    void drawAxisGapAttribute(CCNode* node);
    void drawTextureAttributes(CCNode* node);
    void drawMenuItemAttributes(CCNode* node);
    void drawLayoutOptionsAttributes(CCNode* node);
    void drawLayoutAttributes(CCNode* node);
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

    bool searchBranch(CCNode* node);

    bool hasExtension(const std::string& ext) const;

    DevTools() { loadSettings(); }

public:
    static DevTools* get();
    void loadSettings();
    void saveSettings();
    Settings getSettings();
    bool shouldUseGDWindow() const;

    bool shouldPopGame() const;
    bool pausedGame() const;
    bool isSetup() const;
    bool shouldOrderChildren() const;

    CCNode* getSelectedNode() const;
    void selectNode(CCNode* node);
    void highlightNode(CCNode* node, HighlightMode mode);
    CCNode* getDraggedNode() const;
    void setDraggedNode(CCNode* node);

    void addCustomCallback(std::function<void(CCNode*)> callback);

    void sceneChanged();

    void render(GLRenderCtx* ctx);

    // setup ImGui & DevTools
    void setup();
    void destroy();

    void show(bool visible);
    void toggle();

    bool isVisible();
};
