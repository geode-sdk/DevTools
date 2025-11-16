#define GEODE_DEFINE_EVENT_EXPORTS
#include <API.hpp>
#include "DevTools.hpp"
#include "ImGui.hpp"
#include <misc/cpp/imgui_stdlib.h>

using namespace geode::prelude;

template <typename T>
static void handleType() {
    new EventListener<EventFilter<devtools::PropertyFnEvent<T>>>(+[](devtools::PropertyFnEvent<T>* event) {
        constexpr bool isSigned = std::is_signed_v<T>;
        constexpr ImGuiDataType dataType =
            std::is_same_v<T, float> ? ImGuiDataType_Float :
            std::is_same_v<T, double> ? ImGuiDataType_Double :
            sizeof(T) == 1 ? (isSigned ? ImGuiDataType_S8 : ImGuiDataType_U8) :
            sizeof(T) == 2 ? (isSigned ? ImGuiDataType_S16 : ImGuiDataType_U16) :
            sizeof(T) == 4 ? (isSigned ? ImGuiDataType_S32 : ImGuiDataType_U32) :
            isSigned ? ImGuiDataType_S64 : ImGuiDataType_U64;
        event->fn = +[](const char* name, T& prop) {
            return ImGui::DragScalar(name, dataType, &prop);
        };
        return ListenerResult::Stop;
    });

    new EventListener<EventFilter<devtools::EnumerableFnEvent<T>>>(+[](devtools::EnumerableFnEvent<T>* event) {
        event->fn = +[](const char* label, T* value, std::span<std::pair<T, const char*> const> items) {
            ImGui::Text("%s:", label);
            size_t i = 0;
            bool changed = false;
            for (auto& [itemValue, itemLabel] : items) {
                if (ImGui::RadioButton(itemLabel, *value == itemValue)) {
                    *value = itemValue;
                    changed = true;
                }
                if (i < items.size() - 1) {
                    ImGui::SameLine();
                }
                i++;
            }
            return changed;
        };
        return ListenerResult::Stop;
    });
}

void devtools::newLine() {
    ImGui::NewLine();
}
void devtools::sameLine() {
    ImGui::SameLine();
}
void devtools::separator() {
    ImGui::Separator();
}
void devtools::nextItemWidth(float width) {
    ImGui::SetNextItemWidth(width);
}
bool devtools::combo(char const* label, int& current, std::span<char const*> items, int maxHeight) {
    return ImGui::Combo(
        label,
        &current,
        &*items.begin(),
        static_cast<int>(items.size()),
        maxHeight
    );
}
bool devtools::radio(const char* label, int& current, int num) {
    return ImGui::RadioButton(label, &current, num);
}
void devtools::inputMultiline(const char* label, std::string& str) {
    ImGui::InputTextMultiline(label, &str);
}

$execute {
    new EventListener<EventFilter<devtools::RegisterNodeEvent>>(+[](devtools::RegisterNodeEvent* event) {
        DevTools::get()->addCustomCallback(std::move(event->callback));
        return ListenerResult::Stop;
    });

    // Scalars & Enums
    handleType<char>();
    handleType<unsigned char>();
    handleType<short>();
    handleType<unsigned short>();
    handleType<int>();
    handleType<unsigned int>();
    handleType<long long>();
    handleType<unsigned long long>();
    handleType<long>();
    handleType<unsigned long>();
    handleType<float>();
    handleType<double>();

    // checkbox
    new EventListener<EventFilter<devtools::PropertyFnEvent<bool>>>(+[](devtools::PropertyFnEvent<bool>* event) {
        event->fn = +[](const char* name, bool& prop) {
            return ImGui::Checkbox(name, &prop);
        };
        return ListenerResult::Stop;
    });

    // string
    new EventListener<EventFilter<devtools::PropertyFnEvent<std::string>>>(+[](devtools::PropertyFnEvent<std::string>* event) {
        event->fn = +[](const char* name, std::string& prop) {
            return ImGui::InputText(name, &prop);
        };
        return ListenerResult::Stop;
    });

    // colors
    new EventListener<EventFilter<devtools::PropertyFnEvent<ccColor3B>>>(+[](devtools::PropertyFnEvent<ccColor3B>* event) {
        event->fn = +[](const char* name, ccColor3B& prop) {
            auto color = ImVec4(
                prop.r / 255.f,
                prop.g / 255.f,
                prop.b / 255.f,
                1.0f
            );
            if (ImGui::ColorEdit3(name, &color.x)) {
                prop.r = static_cast<GLubyte>(color.x * 255);
                prop.g = static_cast<GLubyte>(color.y * 255);
                prop.b = static_cast<GLubyte>(color.z * 255);
                return true;
            }
            return false;
        };
        return ListenerResult::Stop;
    });
    new EventListener<EventFilter<devtools::PropertyFnEvent<ccColor4B>>>(+[](devtools::PropertyFnEvent<ccColor4B>* event) {
        event->fn = +[](const char* name, ccColor4B& prop) {
            auto color = ImVec4(
                prop.r / 255.f,
                prop.g / 255.f,
                prop.b / 255.f,
                prop.a / 255.f
            );
            if (ImGui::ColorEdit4(name, &color.x)) {
                prop.r = static_cast<GLubyte>(color.x * 255);
                prop.g = static_cast<GLubyte>(color.y * 255);
                prop.b = static_cast<GLubyte>(color.z * 255);
                prop.a = static_cast<GLubyte>(color.w * 255);
                return true;
            }
            return false;
        };
        return ListenerResult::Stop;
    });
    new EventListener<EventFilter<devtools::PropertyFnEvent<ccColor4F>>>(+[](devtools::PropertyFnEvent<ccColor4F>* event) {
        event->fn = +[](const char* name, ccColor4F& prop) {
            return ImGui::ColorEdit4(name, reinterpret_cast<float*>(&prop));
        };
        return ListenerResult::Stop;
    });

    // points/sizes
    new EventListener<EventFilter<devtools::PropertyFnEvent<CCPoint>>>(+[](devtools::PropertyFnEvent<CCPoint>* event) {
        event->fn = +[](const char* name, CCPoint& prop) {
            return ImGui::DragFloat2(name, reinterpret_cast<float*>(&prop));
        };
        return ListenerResult::Stop;
    });

    new EventListener<EventFilter<devtools::PropertyFnEvent<CCSize>>>(+[](devtools::PropertyFnEvent<CCSize>* event) {
        event->fn = +[](const char* name, CCSize& prop) {
            return ImGui::DragFloat2(name, reinterpret_cast<float*>(&prop));
        };
        return ListenerResult::Stop;
    });
    new EventListener<EventFilter<devtools::PropertyFnEvent<CCRect>>>(+[](devtools::PropertyFnEvent<CCRect>* event) {
        event->fn = +[](const char* name, CCRect& prop) {
            return ImGui::DragFloat4(name, reinterpret_cast<float*>(&prop));
        };
        return ListenerResult::Stop;
    });

    // label
    new EventListener<EventFilter<devtools::DrawLabelFnEvent>>(+[](devtools::DrawLabelFnEvent* event) {
        event->fn = +[](const char* text) {
            ImGui::Text("%s", text);
        };
        return ListenerResult::Stop;
    });

    // button
    new EventListener<EventFilter<devtools::ButtonFnEvent>>(+[](devtools::ButtonFnEvent* event) {
        event->fn = +[](const char* label) {
            return ImGui::Button(label);
        };
        return ListenerResult::Stop;
    });
}