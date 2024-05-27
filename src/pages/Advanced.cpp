#include "../DevTools.hpp"
#include "../ImGui.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <Geode/modify/AppDelegate.hpp>
#include <Geode/loader/Index.hpp>

using namespace geode::prelude;

void DevTools::drawAdvancedSettings() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.f, 1.f });
    ImGui::Checkbox("Show Mod Graph", &m_showModGraph);
    ImGui::Checkbox("Show Mod Index", &m_showModIndex);
    ImGui::PopStyleVar();
}

void DevTools::drawModGraph() {
    // TODO: function to get loader mod
    this->drawModGraphNode(Mod::get()->getMetadata().getDependencies()[0].mod);
}

namespace {
    std::string inputText(const char* label, std::string x) {
        ImGui::InputText(label, &x);
        return x;
    }
    std::optional<std::string> inputText(const char* label, std::optional<std::string> x) {
        std::string str = x ? *x : "";
        return ImGui::InputText(label, &str) ? str : x;
    }
    std::optional<std::string> inputTextMultiline(const char* label, std::optional<std::string> x) {
        std::string str = x ? *x : "";
        return ImGui::InputTextMultiline(label, &str) ? str : x;
    }
    bool inputBool(const char* label, bool x) {
        ImGui::Checkbox(label, &x);
        return x;
    }

    VersionInfo inputVersion(VersionInfo version) {
        int major = (int)version.getMajor();
        int minor = (int)version.getMinor();
        int patch = (int)version.getPatch();
        std::optional<VersionTag> tag = version.getTag();
        ImGui::InputInt("version.major", &major);
        ImGui::InputInt("version.minor", &minor);
        ImGui::InputInt("version.patch", &patch);
        int tagSel = tag ? tag->value + 1 : 0;
        ImGui::Combo("version.tag.value", &tagSel, "std::monostate\0Alpha\0Beta\0Prerelease\0\0");
        if (tagSel == 0)
            tag.reset();
        else {
            int tagNum = tag && tag->number ? (int)*tag->number : -1;
            ImGui::InputInt("version.tag.number", &tagNum);
            auto t = (VersionTag::Type)(tagSel - 1);
            if (tagNum < 0)
                tag = VersionTag(t);
            else
                tag = VersionTag(t, tagNum);
        }
        return {(size_t)major, (size_t)minor, (size_t)patch, tag};
    }

    std::optional<ModMetadata::IssuesInfo> inputIssues(std::optional<ModMetadata::IssuesInfo> x) {
        ModMetadata::IssuesInfo a = x ? *x : ModMetadata::IssuesInfo{"", std::nullopt};
        std::string url = a.url ? *a.url : "";
        bool inputInfo = ImGui::InputText("issues.info", &a.info);
        bool inputUrl = ImGui::InputText("issues.url", &url);
        if (inputUrl)
            a.url = url;
        return inputInfo || inputUrl ? a : x;
    }
}

ModMetadata DevTools::inputMetadata(void* treePtr, ModMetadata metadata) {
    metadata.setVersion(inputVersion(metadata.getVersion()));
    metadata.setName(inputText("name", metadata.getName()));
    metadata.setDeveloper(inputText("developer", metadata.getDevelopers()[0]));
    metadata.setDescription(inputTextMultiline("description", metadata.getDescription()));
    metadata.setDetails(inputTextMultiline("details", metadata.getDetails()));
    metadata.setChangelog(inputTextMultiline("changelog", metadata.getChangelog()));
    metadata.setSupportInfo(inputTextMultiline("supportInfo", metadata.getSupportInfo()));
    metadata.setRepository(inputTextMultiline("repository", metadata.getRepository()));
    metadata.setIssues(inputIssues(metadata.getIssues()));
    metadata.setNeedsEarlyLoad(inputBool("needsEarlyLoad", metadata.needsEarlyLoad()));
    metadata.setIsAPI(inputBool("isAPI", metadata.isAPI()));

    if (ImGui::TreeNode(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(treePtr) + 1), "dependencies")) {
        for (auto const& item : metadata.getDependencies()) {
            if (item.mod) {
                if (!ImGui::TreeNode(item.mod, "%s", item.id.c_str()))
                    continue;
            }
            else {
                if (!ImGui::TreeNode(item.id.data(), "%s", item.id.c_str()))
                    continue;
            }
            ImGui::Text("version: %s", item.version.toString().c_str());
            const char* importance = "";
            switch (item.importance) {
                case geode::ModMetadata::Dependency::Importance::Required: importance = "required"; break;
                case geode::ModMetadata::Dependency::Importance::Recommended: importance = "recommended"; break;
                case geode::ModMetadata::Dependency::Importance::Suggested: importance = "suggested"; break;
            }
            ImGui::Text("importance: %s", importance);
            ImGui::Text("isResolved: %s", item.isResolved() ? "true" : "false");
            if (item.mod)
                drawModGraphNode(item.mod);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(treePtr) + 2), "incompatibilities")) {
        for (auto const& item : metadata.getIncompatibilities()) {
            if (item.mod) {
                if (!ImGui::TreeNode(item.mod, "%s", item.id.c_str()))
                    continue;
            }
            else {
                if (!ImGui::TreeNode(item.id.data(), "%s", item.id.c_str()))
                    continue;
            }
            ImGui::Text("version: %s", item.version.toString().c_str());
            const char* importance = "";
            switch (item.importance) {
                case geode::ModMetadata::Incompatibility::Importance::Breaking: importance = "breaking"; break;
                case geode::ModMetadata::Incompatibility::Importance::Conflicting: importance = "conflicting"; break;
                case geode::ModMetadata::Incompatibility::Importance::Superseded: importance = "superseded"; break;
            }
            ImGui::Text("importance: %s", importance);
            ImGui::Text("isResolved: %s", item.isResolved() ? "true" : "false");
            if (item.mod)
                drawModGraphNode(item.mod);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(treePtr) + 3), "spritesheets")) {
        for (auto const& item : metadata.getSpritesheets()) {
            ImGui::Text("%s", item.c_str());
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(treePtr) + 4), "settings")) {
        for (auto const& [id, setting] : metadata.getSettings()) {
            if (!ImGui::TreeNode(id.data(), "%s", id.c_str()))
                continue;
            ImGui::Text("displayName: %s", setting.getDisplayName().c_str());
            if (setting.getDescription())
                ImGui::Text("description: %s", setting.getDescription()->c_str());
            ImGui::Text("isCustom: %s", setting.isCustom() ? "true" : "false");
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    return metadata;
}

void DevTools::drawModGraphNode(Mod* node) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

    ImColor color = ImColor(1.f, 1.f, 1.f);
    if (node->isUninstalled())
        color = ImColor(0.1f, 0.1f, 0.1f);
    else if (!node->isEnabled())
        color = ImColor(0.7f, 0.7f, 0.7f);

    ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)color);
    auto treeNode = ImGui::TreeNodeEx(node, flags, "%s", node->getID().c_str());
    ImGui::PopStyleColor();

    if (!treeNode)
        return;

    node->setMetadata(this->inputMetadata(node, node->getMetadata()));

    ImGui::Text("isInternal: %s", node->isInternal() ? "true" : "false");
    ImGui::Text("early: %s", node->needsEarlyLoad() ? "true" : "false");
    ImGui::Text("hasUnresolvedDependencies: %s", node->hasUnresolvedDependencies() ? "true" : "false");
    ImGui::Text("hasUnresolvedIncompatibilities: %s", node->hasUnresolvedIncompatibilities() ? "true" : "false");

    for (auto& dep : node->getDependants()) {
        this->drawModGraphNode(dep);
    }

    ImGui::TreePop();
}

void DevTools::drawModIndex() {
    for (auto const& item : Index::get()->getItems()) {
        drawIndexItem(item);
    }
}

void DevTools::drawIndexItem(IndexItemHandle const& node) {
    auto* item = node.get();
    if (!item || !ImGui::TreeNode(item, "%s", item->getMetadata().getID().c_str()))
        return;
    item->setMetadata(this->inputMetadata(item, item->getMetadata()));
    item->setDownloadURL(inputText("downloadURL", item->getDownloadURL()));
    item->setPackageHash(inputText("packageHash", item->getPackageHash()));
    if (ImGui::TreeNode(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(item) + 5), "availablePlatforms")) {
        auto platforms = item->getAvailablePlatforms();
        for (PlatformID::Type type = PlatformID::Type::Unknown; type <= PlatformID::Type::Linux; (*(int*)&type)++) {
            bool contains = platforms.contains({type});
            if (!ImGui::Checkbox(PlatformID::toString(type), &contains))
                continue;
            if (contains)
                platforms.insert({type});
            else
                platforms.erase({type});
        }
        item->setAvailablePlatforms(platforms);
        ImGui::TreePop();
    }
    item->setIsFeatured(inputBool("isFeatured", item->isFeatured()));
    if (ImGui::TreeNode(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(item) + 6), "tags")) {
        auto tags = item->getTags();
        static std::string current;
        ImGui::InputText("", &current);
        ImGui::SameLine();
        if (ImGui::Button("Add")) {
            tags.insert(current);
            current = "";
        }
        for (auto const& tag : item->getTags()) {
            ImGui::Text("%s", tag.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
                tags.erase(tag);
        }
        item->setTags(tags);
        ImGui::TreePop();
    }
    ImGui::Text("isInstalled: %s", item->isInstalled() ? "true" : "false");
    ImGui::TreePop();
}
