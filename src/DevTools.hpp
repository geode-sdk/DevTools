#pragma once

#include "platform/platform.hpp"
#include <imgui.h>
#include "themes.hpp"
#include <cocos2d.h>
#include <Geode/utils/cocos.hpp>
#include <unordered_map>

USE_GEODE_NAMESPACE();

enum class HighlightMode {
    Selected,
    Hovered,
};

class DevTools {
protected:
    bool m_visible = false;
    bool m_setup = false;
    bool m_reloadTheme = true;
    bool m_GDInWindow = true;
    bool m_attributesInTree = false;
    bool m_alwaysHighlight = true;
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
    void drawNodeAttributes(CCNode* node);
    void drawAttributes();
    void drawLayout();
    void drawNodeLayout(CCNode* node);
    void drawHighlight(CCNode* node, HighlightMode mode);
    void drawGD(GLRenderCtx* ctx);
    void drawPage(const char* name, void(DevTools::* fun)());
    void drawPages();
    void draw(GLRenderCtx* ctx);

    void newFrame();
    void renderDrawData(ImDrawData*);

public:
    static DevTools* get();

    bool shouldPopGame() const;

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
