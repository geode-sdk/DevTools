#include <API.hpp>
#include "DevTools.hpp"
#include "ImGui.hpp"
#include <misc/cpp/imgui_stdlib.h>

using namespace geode::prelude;

template <typename T>
static void handleType() {
    new EventListener<EventFilter<devtools::HandlePropertyEvent<T>>>(+[](devtools::HandlePropertyEvent<T>* event) {
        constexpr bool isSigned = std::is_signed_v<T>;
        constexpr ImGuiDataType dataType = sizeof(T) == 1 ? (isSigned ? ImGuiDataType_S8 : ImGuiDataType_U8) :
                                           sizeof(T) == 2 ? (isSigned ? ImGuiDataType_S16 : ImGuiDataType_U16) :
                                           sizeof(T) == 4 ? (isSigned ? ImGuiDataType_S32 : ImGuiDataType_U32) :
                                           isSigned ? ImGuiDataType_S64 : ImGuiDataType_U64;
        event->changed = ImGui::InputScalar(event->name, dataType, event->prop);
        return ListenerResult::Stop;
    });

    new EventListener<EventFilter<devtools::EnumerableEvent<T>>>(+[](devtools::EnumerableEvent<T>* event) {
        ImGui::Text("%s:", event->label);
        size_t i = 0;
        for (auto& [value, label] : event->items) {
            if (ImGui::RadioButton(label, *event->value == value)) {
                *event->value = value;
                event->changed = true;
            }
            if (i < event->items.size() - 1) {
                ImGui::SameLine();
            }
            i++;
        }
        return ListenerResult::Stop;
    });
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

    // checkbox
    new EventListener<EventFilter<devtools::HandlePropertyEvent<bool>>>(+[](devtools::HandlePropertyEvent<bool>* event) {
        event->changed = ImGui::Checkbox(event->name, event->prop);
        return ListenerResult::Stop;
    });

    // float and double
    new EventListener<EventFilter<devtools::HandlePropertyEvent<float>>>(+[](devtools::HandlePropertyEvent<float>* event) {
        event->changed = ImGui::InputFloat(event->name, event->prop);
        return ListenerResult::Stop;
    });
    new EventListener<EventFilter<devtools::HandlePropertyEvent<double>>>(+[](devtools::HandlePropertyEvent<double>* event) {
        event->changed = ImGui::InputDouble(event->name, event->prop);
        return ListenerResult::Stop;
    });

    // string
    new EventListener<EventFilter<devtools::HandlePropertyEvent<std::string>>>(+[](devtools::HandlePropertyEvent<std::string>* event) {
        event->changed = ImGui::InputText(event->name, event->prop);
        return ListenerResult::Stop;
    });

    // colors
    new EventListener<EventFilter<devtools::HandlePropertyEvent<ccColor3B>>>(+[](devtools::HandlePropertyEvent<ccColor3B>* event) {
        auto color = ImVec4(
            event->prop->r / 255.f,
            event->prop->g / 255.f,
            event->prop->b / 255.f,
            1.0f
        );
        if (ImGui::ColorEdit3(event->name, &color.x)) {
            event->changed = true;
            event->prop->r = static_cast<GLubyte>(color.x * 255);
            event->prop->g = static_cast<GLubyte>(color.y * 255);
            event->prop->b = static_cast<GLubyte>(color.z * 255);
        }
        return ListenerResult::Stop;
    });
    new EventListener<EventFilter<devtools::HandlePropertyEvent<ccColor4B>>>(+[](devtools::HandlePropertyEvent<ccColor4B>* event) {
        auto color = ImVec4(
            event->prop->r / 255.f,
            event->prop->g / 255.f,
            event->prop->b / 255.f,
            event->prop->a / 255.f
        );
        if (ImGui::ColorEdit4(event->name, &color.x)) {
            event->changed = true;
            event->prop->r = static_cast<GLubyte>(color.x * 255);
            event->prop->g = static_cast<GLubyte>(color.y * 255);
            event->prop->b = static_cast<GLubyte>(color.z * 255);
            event->prop->a = static_cast<GLubyte>(color.w * 255);
        }
        return ListenerResult::Stop;
    });
    new EventListener<EventFilter<devtools::HandlePropertyEvent<ccColor4F>>>(+[](devtools::HandlePropertyEvent<ccColor4F>* event) {
        event->changed = ImGui::ColorEdit4(event->name, reinterpret_cast<float*>(event->prop));
        return ListenerResult::Stop;
    });

    // points/sizes
    new EventListener<EventFilter<devtools::HandlePropertyEvent<CCPoint>>>(+[](devtools::HandlePropertyEvent<CCPoint>* event) {
        event->changed = ImGui::InputFloat2(event->name, reinterpret_cast<float*>(event->prop));
        return ListenerResult::Stop;
    });
    new EventListener<EventFilter<devtools::HandlePropertyEvent<CCSize>>>(+[](devtools::HandlePropertyEvent<CCSize>* event) {
        event->changed = ImGui::InputFloat2(event->name, reinterpret_cast<float*>(event->prop));
        return ListenerResult::Stop;
    });
    new EventListener<EventFilter<devtools::HandlePropertyEvent<CCRect>>>(+[](devtools::HandlePropertyEvent<CCRect>* event) {
        event->changed = ImGui::InputFloat4(event->name, reinterpret_cast<float*>(event->prop));
        return ListenerResult::Stop;
    });

    // label
    new EventListener<EventFilter<devtools::DrawLabelEvent>>(+[](devtools::DrawLabelEvent* event) {
        ImGui::Text("%s", event->text);
        return ListenerResult::Stop;
    });

    // button
    new EventListener<EventFilter<devtools::ButtonEvent>>(+[](devtools::ButtonEvent* event) {
        event->clicked = ImGui::Button(event->label);
        return ListenerResult::Stop;
    });
}