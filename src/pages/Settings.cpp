
#include "../fonts/FeatherIcons.hpp"
#include "../DevTools.hpp"
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/ranges.hpp>
#include <numeric>

USE_GEODE_NAMESPACE();

static float RAINBOW_HUE = 0.f;

void DevTools::drawSettings() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.f, 1.f });
    ImGui::Checkbox("GD in Window", &m_GDInWindow);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Show GD inside a window when DevTools are open");
    }
    ImGui::Checkbox("Attributes in Tree", &m_attributesInTree);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Show node attributes in the Tree");
    }
    ImGui::Checkbox("Highlight Nodes", &m_alwaysHighlight);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Always highlight nodes when hovered in the Tree. "
            "When disabled, you can highlight by pressing Shift."
        );
    }
    ImGui::Checkbox("Highlight Layouts", &m_highlightLayouts);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Highlights the borders of all layouts applied to nodes"
        );
    }
    ImGui::PopStyleVar();

    ImGui::Separator();

    ImGui::Text("GD Window");

    auto winSize = CCDirector::get()->getWinSize();
    auto frameSize = GameManager::get()->resolutionForKey(GameManager::get()->m_resolution);
    auto fps = roundf(1 / CCDirector::get()->getAnimationInterval());
    auto ratio = std::gcd(static_cast<int>(frameSize.width), static_cast<int>(frameSize.height));

    std::string text = "";
    text += "Custom";
    text.push_back('\0');
    for (int i = 1; i < 28; i++) {
        auto size = GameManager::get()->resolutionForKey(i);
        text += fmt::format("{}x{}", size.width, size.height);
        text.push_back('\0');
    }
    int selectedResolution = GameManager::get()->m_resolution;

    static CCSize customResolution = frameSize;

    if (ImGui::Combo("##devtools/resolution", &selectedResolution, text.c_str())) {
        GameManager::get()->m_resolution = selectedResolution;

        // TODO: idk how to do this on macos
    #ifdef GEODE_IS_WINDOWS
        if (selectedResolution != 0) {
            auto size = GameManager::get()->resolutionForKey(selectedResolution);
            CCEGLView::get()->resizeWindow(size.width, size.height);
        }
        else {
            CCEGLView::get()->resizeWindow(customResolution.width, customResolution.height);
        }
        CCEGLView::get()->centerWindow();
    #endif
    }

    if (selectedResolution == 0) {
        int size[2] = {
            static_cast<int>(customResolution.width),
            static_cast<int>(customResolution.height),
        };
        if (ImGui::DragInt2("Size", size)) {
            size[0] = std::fabs(size[0]);
            size[1] = std::fabs(size[1]);
            customResolution = CCSizeMake(size[0], size[1]);
        }
    #ifdef GEODE_IS_WINDOWS
        if (ImGui::Button("Apply##size-apply")) {
            GameManager::get()->m_resolution = 0;
            CCEGLView::get()->resizeWindow(customResolution.width, customResolution.height);
            CCEGLView::get()->centerWindow();
        }
    #endif
    }

    ImGui::TextWrapped(
        "GL Size: %dx%d",
        static_cast<int>(winSize.width),
        static_cast<int>(winSize.height)
    );
    ImGui::TextWrapped(
        "Frame Size: %dx%d",
        static_cast<int>(frameSize.width),
        static_cast<int>(frameSize.height)
    );
    ImGui::TextWrapped("FPS: %d", static_cast<int>(fps));
    ImGui::TextWrapped(
        "Aspect Ratio: %d:%d",
        static_cast<int>(frameSize.width / ratio),
        static_cast<int>(frameSize.height / ratio)
    );

    ImGui::Separator();

    ImGui::Text("Theme");
    static auto SELECTED = static_cast<int>(getThemeIndex(m_theme));
    if (ImGui::Combo("##devtools/theme", &SELECTED,
        (ranges::join(getThemeOptions(), std::string(1, '\0')) + '\0').c_str()
    )) {
        m_theme = getThemeAtIndex(SELECTED);
        m_reloadTheme = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Select Theme");
    }

    ImGui::Separator();

    ImGui::TextWrapped("Developed by ");

    RAINBOW_HUE += 0.01f;
    if (RAINBOW_HUE >= 1.f) {
        RAINBOW_HUE = 0.f;
    }

    float hue = RAINBOW_HUE;

    ImVec4 color;
    color.w = 1.f;
    for (auto c : "Geode Team") {
        hue += 0.04f;
        ImGui::SameLine(0.f, 0.f);
        ImGui::ColorConvertHSVtoRGB(hue, .5f, 1.f, color.x, color.y, color.z);
        ImGui::TextColored(color, std::string(1, c).c_str());
    }

    ImGui::TextWrapped(
        "Running Geode %s, DevTools %s",
        Loader::get()->getVersion().toString().c_str(),
        Mod::get()->getVersion().toString().c_str()
    );

    if (ImGui::Button("Reset Layout")) {
        m_shouldRelayout = true;
    }
}
