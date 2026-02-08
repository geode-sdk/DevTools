#define GEODE_DEFINE_EVENT_EXPORTS
#include <API.hpp>
#include "DevTools.hpp"
#include "ImGui.hpp"
#include <misc/cpp/imgui_stdlib.h>

using namespace geode::prelude;

template <typename T>
static void handleType() {
    devtools::PropertyFnEvent<T>().listen([](typename devtools::PropertyFnEvent<T>::Fn*& fnPtr) {
        constexpr bool isSigned = std::is_signed_v<T>;
        constexpr ImGuiDataType dataType =
            std::is_same_v<T, float> ? ImGuiDataType_Float :
            std::is_same_v<T, double> ? ImGuiDataType_Double :
            sizeof(T) == 1 ? (isSigned ? ImGuiDataType_S8 : ImGuiDataType_U8) :
            sizeof(T) == 2 ? (isSigned ? ImGuiDataType_S16 : ImGuiDataType_U16) :
            sizeof(T) == 4 ? (isSigned ? ImGuiDataType_S32 : ImGuiDataType_U32) :
            isSigned ? ImGuiDataType_S64 : ImGuiDataType_U64;
        fnPtr = +[](ZStringView name, T& prop) {
            return ImGui::DragScalar(name.c_str(), dataType, &prop);
        };
        return ListenerResult::Stop;
    }).leak();

    devtools::EnumerableFnEvent<T>().listen([](typename devtools::EnumerableFnEvent<T>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView label, T* value, std::span<std::pair<T, ZStringView> const> items) {
            ImGui::Text("%s:", label.c_str());
            size_t i = 0;
            bool changed = false;
            for (auto& [itemValue, itemLabel] : items) {
                if (ImGui::RadioButton(itemLabel.c_str(), *value == itemValue)) {
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
    }).leak();
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
void devtools::indent() {
    ImGui::Indent(16.f);
}
void devtools::unindent() {
    ImGui::Unindent(16.f);
}
bool devtools::combo(ZStringView label, int& current, std::span<char const*> items, int maxHeight) {
    return ImGui::Combo(
        label.c_str(),
        &current,
        &*items.begin(),
        static_cast<int>(items.size()),
        maxHeight
    );
}
bool devtools::radio(ZStringView label, int& current, int num) {
    return ImGui::RadioButton(label.c_str(), &current, num);
}
void devtools::inputMultiline(ZStringView label, std::string& str) {
    ImGui::InputTextMultiline(label.c_str(), &str);
}

void devtools::label(ZStringView text) {
    ImGui::Text("%s", text.c_str());
}

bool devtools::button(ZStringView label) {
    return ImGui::Button(label.c_str());
}

$execute {
    devtools::RegisterNodeEvent().listen([](Function<void(CCNode*)> callback) {
        DevTools::get()->addCustomCallback(std::move(callback));
        return ListenerResult::Stop;
    }).leak();

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
    devtools::PropertyFnEvent<bool>().listen([](devtools::PropertyFnEvent<bool>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView name, bool& prop) {
            return ImGui::Checkbox(name.c_str(), &prop);
        };
        return ListenerResult::Stop;
    }).leak();

    // string
    devtools::PropertyFnEvent<std::string>().listen([](devtools::PropertyFnEvent<std::string>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView name, std::string& prop) {
            return ImGui::InputText(name.c_str(), &prop);
        };
        return ListenerResult::Stop;
    }).leak();

    // colors
    devtools::PropertyFnEvent<ccColor3B>().listen([](devtools::PropertyFnEvent<ccColor3B>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView name, ccColor3B& prop) {
            auto color = ImVec4(
                prop.r / 255.f,
                prop.g / 255.f,
                prop.b / 255.f,
                1.0f
            );
            if (ImGui::ColorEdit3(name.c_str(), &color.x)) {
                prop.r = static_cast<GLubyte>(color.x * 255);
                prop.g = static_cast<GLubyte>(color.y * 255);
                prop.b = static_cast<GLubyte>(color.z * 255);
                return true;
            }
            return false;
        };
        return ListenerResult::Stop;
    }).leak();
    devtools::PropertyFnEvent<ccColor4B>().listen([](devtools::PropertyFnEvent<ccColor4B>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView name, ccColor4B& prop) {
            auto color = ImVec4(
                prop.r / 255.f,
                prop.g / 255.f,
                prop.b / 255.f,
                prop.a / 255.f
            );
            if (ImGui::ColorEdit4(name.c_str(), &color.x)) {
                prop.r = static_cast<GLubyte>(color.x * 255);
                prop.g = static_cast<GLubyte>(color.y * 255);
                prop.b = static_cast<GLubyte>(color.z * 255);
                prop.a = static_cast<GLubyte>(color.w * 255);
                return true;
            }
            return false;
        };
        return ListenerResult::Stop;
    }).leak();
    devtools::PropertyFnEvent<ccColor4F>().listen([](devtools::PropertyFnEvent<ccColor4F>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView name, ccColor4F& prop) {
            return ImGui::ColorEdit4(name.c_str(), reinterpret_cast<float*>(&prop));
        };
        return ListenerResult::Stop;
    }).leak();

    // points/sizes
    devtools::PropertyFnEvent<CCPoint>().listen([](devtools::PropertyFnEvent<CCPoint>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView name, CCPoint& prop) {
            return ImGui::DragFloat2(name.c_str(), reinterpret_cast<float*>(&prop));
        };
        return ListenerResult::Stop;
    }).leak();

    devtools::PropertyFnEvent<CCSize>().listen([](devtools::PropertyFnEvent<CCSize>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView name, CCSize& prop) {
            return ImGui::DragFloat2(name.c_str(), reinterpret_cast<float*>(&prop));
        };
        return ListenerResult::Stop;
    }).leak();
    devtools::PropertyFnEvent<CCRect>().listen([](devtools::PropertyFnEvent<CCRect>::Fn*& fnPtr) {
        fnPtr = +[](ZStringView name, CCRect& prop) {
            return ImGui::DragFloat4(name.c_str(), reinterpret_cast<float*>(&prop));
        };
        return ListenerResult::Stop;
    }).leak();
}