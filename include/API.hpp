#pragma once
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <cocos2d.h>
#include <functional>
#include <initializer_list>
#include <string>
#include <type_traits>

namespace devtools {
    template <typename T>
    concept IsCCNode = std::is_base_of_v<cocos2d::CCNode, std::remove_pointer_t<T>>;

    template <typename T>
    concept SupportedProperty = std::is_arithmetic_v<T> ||
                                std::is_same_v<T, std::string> ||
                                std::is_same_v<T, cocos2d::ccColor3B> ||
                                std::is_same_v<T, cocos2d::ccColor4B> ||
                                std::is_same_v<T, cocos2d::ccColor4F> ||
                                std::is_same_v<T, cocos2d::CCPoint> ||
                                std::is_same_v<T, cocos2d::CCSize> ||
                                std::is_same_v<T, cocos2d::CCRect>;

    struct RegisterNodeEvent final : geode::Event {
        RegisterNodeEvent(std::function<void(cocos2d::CCNode*)>&& callback)
            : callback(std::move(callback)) {}
        std::function<void(cocos2d::CCNode*)> callback;
    };

    template <typename T>
    struct HandlePropertyEvent final : geode::Event {
        HandlePropertyEvent(const char* n, T* p) : prop(p), name(n) {}
        T* prop;
        const char* name = nullptr;
        bool changed = false;
    };

    struct DrawLabelEvent final : geode::Event {
        DrawLabelEvent(const char* t) : text(t) {}
        const char* text = nullptr;
    };

    template <typename T>
    struct EnumerableEvent final : geode::Event {
        EnumerableEvent(const char* l, T* v, std::initializer_list<std::pair<T, const char*>> i)
            : label(l), value(v), items(i) {}
        const char* label = nullptr;
        T* value;
        std::initializer_list<std::pair<T, const char*>> items;
        bool changed = false;
    };

    struct ButtonEvent final : geode::Event {
        ButtonEvent(const char* l) : label(l) {}
        const char* label = nullptr;
        bool clicked = false;
    };

    /// @brief Checks if DevTools is currently loaded.
    /// @return True if DevTools is loaded, false otherwise.
    inline bool isLoaded() {
        return geode::Loader::get()->getLoadedMod("geode.devtools") != nullptr;
    }

    /// @brief Waits for DevTools to be loaded and then calls the provided callback.
    /// @param callback The function to call once DevTools is loaded.
    template <typename F>
    void waitForDevTools(F&& callback) {
        if (isLoaded()) {
            callback();
        } else {
            auto devtools = geode::Loader::get()->getInstalledMod("geode.devtools");
            if (!devtools || !devtools->isEnabled()) return;

            new geode::EventListener(
                [callback = std::forward<F>(callback)](geode::ModStateEvent*) {
                    callback();
                },
                geode::ModStateFilter(devtools, geode::ModEventType::Loaded)
            );
        }
    }

    /// @brief Registers a callback that will be called whenever a node of type T is opened in Attributes tab.
    /// @param callback The function to call with the node when it is opened.
    /// @see `devtools::property`, `devtools::label`, `devtools::enumerable`, `devtools::button`
    template <typename T, std::invocable<std::remove_pointer_t<T>*> F> requires IsCCNode<T>
    void registerNode(F&& callback) {
        RegisterNodeEvent([callback = std::forward<F>(callback)](cocos2d::CCNode* node) {
            if (auto casted = geode::cast::typeinfo_cast<std::remove_pointer_t<T>*>(node)) {
                callback(casted);
            }
        }).post();
    }

    /// @brief Renders a property editor for the given value in the DevTools UI.
    /// @param name The name of the property to display.
    /// @param prop The property value to edit.
    /// @return True if the property was changed, false otherwise.
    /// @warning This function should only ever be called from within a registered node callback.
    template <typename T> requires SupportedProperty<T>
    bool property(const char* name, T& prop) {
        HandlePropertyEvent event(name, &prop);
        event.post();
        return event.changed;
    }

    /// @brief Renders a label in the DevTools UI.
    /// @param text The text to display in the label.
    /// @warning This function should only ever be called from within a registered node callback.
    inline void label(const char* text) {
        DrawLabelEvent(text).post();
    }

    /// @brief Renders an enumerable property editor using radio buttons for the given value in the DevTools UI.
    /// @param label The label for the enumerable property.
    /// @param value The value to edit, which should be an enum or integral type.
    /// @param items A list of pairs where each pair contains a value and its corresponding label.
    /// @return True if the value was changed, false otherwise.
    /// @warning This function should only ever be called from within a registered node callback.
    template <typename T> requires std::is_integral_v<std::underlying_type_t<T>>
    bool enumerable(const char* label, T& value, std::initializer_list<std::pair<T, const char*>> items) {
        using ValueType = std::underlying_type_t<T>;
        EnumerableEvent<ValueType> event(
            label, reinterpret_cast<ValueType*>(&value),
            *reinterpret_cast<std::initializer_list<std::pair<ValueType, const char*>>*>(&items)
        );
        event.post();
        return event.changed;
    }

    /// @brief Renders a button in the DevTools UI.
    /// @param label The label for the button.
    /// @return True if the button was clicked, false otherwise.
    /// @warning This function should only ever be called from within a registered node callback.
    inline bool button(const char* label) {
        ButtonEvent event(label);
        event.post();
        return event.clicked;
    }

    /// @brief Renders a button in the DevTools UI and calls the provided callback if the button is clicked.
    /// @param label The label for the button.
    /// @param callback The function to call when the button is clicked.
    /// @warning This function should only ever be called from within a registered node callback.
    template <typename F>
    void button(const char* label, F&& callback) {
        if (button(label)) {
            callback();
        }
    }
}