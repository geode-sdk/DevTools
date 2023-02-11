
#include "../fonts/FeatherIcons.hpp"
#include "../DevTools.hpp"
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/ranges.hpp>

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

    ImGui::Text("Theme");
    static auto SELECTED = static_cast<int>(getThemeIndex(m_theme));
    if (ImGui::Combo("##dev.theme", &SELECTED,
        (ranges::join(getThemeOptions(), std::string(1, '\0')) + '\0').c_str()
    )) {
        m_theme = getThemeAtIndex(SELECTED);
        m_reloadTheme = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Select Theme");
    }

    ImGui::Separator();

    ImGui::TextWrapped("Developed by HJfod");
    ImGui::TextWrapped("Original by");
    ImGui::SameLine();

    RAINBOW_HUE += 0.01f;
    if (RAINBOW_HUE >= 1.f) {
        RAINBOW_HUE = 0.f;
    }

    ImVec4 color;
    color.w = 1.f;
    ImGui::ColorConvertHSVtoRGB(RAINBOW_HUE, 1.f, 1.f, color.x, color.y, color.z);
    ImGui::TextColored(color, "Mat");

    ImGui::Separator();

    ImGui::TextWrapped(
        "Running Geode %s, DevTools %s",
        Loader::get()->getVersion().toString().c_str(),
        Mod::get()->getVersion().toString().c_str()
    );

    if (ImGui::Button("Reset Layout")) {
        m_shouldRelayout = true;
    }
}
