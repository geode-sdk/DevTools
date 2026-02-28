#pragma once
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <cocos2d.h>
#include <functional>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <Geode/loader/Dispatch.hpp>
#define MY_MOD_ID "geode.devtools"

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

    template <typename T>
    concept UnderlyingIntegral = std::is_integral_v<T> || std::is_integral_v<std::underlying_type_t<T>>;

    struct RegisterNodeEvent final : geode::Event<RegisterNodeEvent, bool(geode::Function<void(cocos2d::CCNode*)>&)> {
        using Event::Event;
    };

    template <typename T>
    struct PropertyFnEvent final : geode::Event<PropertyFnEvent<T>, bool(bool(*&)(geode::ZStringView name, T&))> {
        using Fn = bool(geode::ZStringView name, T&);
        using geode::Event<PropertyFnEvent, bool(Fn*&)>::Event;
    };

    template <typename T>
    struct EnumerableFnEvent final : geode::Event<EnumerableFnEvent<T>, bool(bool(*&)(geode::ZStringView label, T* value, std::span<std::pair<T, geode::ZStringView> const>))> {
        using Fn = bool(geode::ZStringView label, T* value, std::span<std::pair<T, geode::ZStringView> const>);
        using geode::Event<EnumerableFnEvent, bool(Fn*&)>::Event;
    };

    /// @brief Checks if DevTools is currently loaded.
    /// @return True if DevTools is loaded, false otherwise.
    inline bool isLoaded() {
        return geode::Loader::get()->getLoadedMod("geode.devtools") != nullptr;
    }

    /// @brief Checks if DevTools is currently open.
    /// @return True if DevTools is open, false otherwise.
    inline bool isOpen() GEODE_EVENT_EXPORT_NORES(&isOpen, ());

    /// @brief Waits for DevTools to be loaded and then calls the provided callback.
    /// @param callback The function to call once DevTools is loaded.
    template <typename F>
    void waitForDevTools(F&& callback) {
        if (isLoaded()) {
            callback();
        } else {
            auto devtools = geode::Loader::get()->getInstalledMod("geode.devtools");
            if (!devtools) return;

            geode::ModStateEvent(geode::ModEventType::Loaded, devtools).listen(
                [callback = std::forward<F>(callback)]() {
                    callback();
                }
            ).leak();
        }
    }

    /// @brief Registers a callback that will be called whenever a node of type T is opened in Attributes tab.
    /// @param callback The function to call with the node when it is opened.
    /// @see `devtools::property`, `devtools::label`, `devtools::enumerable`, `devtools::button`
    template <typename T, std::invocable<std::remove_pointer_t<T>*> F> requires IsCCNode<T>
    void registerNode(F&& callback) {
        geode::Function<void(cocos2d::CCNode*)> func = [callback = std::forward<F>(callback)](cocos2d::CCNode* node) {
            if (auto casted = geode::cast::typeinfo_cast<std::remove_pointer_t<T>*>(node)) {
                callback(casted);
            }
        };

        RegisterNodeEvent().send(func);
    }

    /// @brief Renders a property editor for the given value in the DevTools UI.
    /// @param name The name of the property to display.
    /// @param prop The property value to edit.
    /// @return True if the property was changed, false otherwise.
    /// @warning This function should only ever be called from within a registered node callback.
    template <typename T> requires SupportedProperty<T>
    bool property(geode::ZStringView name, T& prop) {
        static auto fn = ([] {
            typename PropertyFnEvent<T>::Fn* fnPtr = nullptr;
            PropertyFnEvent<T>().send(fnPtr);
            return fnPtr;
        })();
        return fn ? fn(name, prop) : false;
    }

    /// @brief Renders a label in the DevTools UI.
    /// @param text The text to display in the label.
    /// @warning This function should only ever be called from within a registered node callback.
    inline void label(geode::ZStringView text) GEODE_EVENT_EXPORT_NORES(&label, (text));

    /// @brief Renders an enumerable property editor using radio buttons for the given value in the DevTools UI.
    /// @param label The label for the enumerable property.
    /// @param value The value to edit, which should be an enum or integral type.
    /// @param items A list of pairs where each pair contains a value and its corresponding label.
    /// @return True if the value was changed, false otherwise.
    /// @warning This function should only ever be called from within a registered node callback.
    template <UnderlyingIntegral T>
    bool enumerable(geode::ZStringView label, T& value, std::initializer_list<std::pair<T, geode::ZStringView>> items) {
        using ValueType = std::underlying_type_t<T>;
        static auto fn = ([] {
            typename EnumerableFnEvent<ValueType>::Fn* fnPtr = nullptr;
            EnumerableFnEvent<ValueType>().send(fnPtr);
            return fnPtr;
        })();
        return fn ? fn(
            label,
            reinterpret_cast<ValueType*>(&value),
            std::span(
                reinterpret_cast<std::pair<ValueType, geode::ZStringView> const*>(&*items.begin()),
                reinterpret_cast<std::pair<ValueType, geode::ZStringView> const*>(&*items.end())
            )
        ) : false;
    }

    /// @brief Renders a button in the DevTools UI.
    /// @param label The label for the button.
    /// @return True if the button was clicked, false otherwise.
    /// @warning This function should only ever be called from within a registered node callback.
    inline bool button(geode::ZStringView label) GEODE_EVENT_EXPORT_NORES(&button, (label));

    /// @brief Renders a button in the DevTools UI and calls the provided callback if the button is clicked.
    /// @param label The label for the button.
    /// @param callback The function to call when the button is clicked.
    /// @warning This function should only ever be called from within a registered node callback.
    template <typename F>
    void button(geode::ZStringView label, F&& callback) {
        if (button(label)) {
            callback();
        }
    }

    inline void newLine() GEODE_EVENT_EXPORT_NORES(&newLine, ());
    inline void sameLine() GEODE_EVENT_EXPORT_NORES(&sameLine, ());
    inline void separator() GEODE_EVENT_EXPORT_NORES(&separator, ());
    inline void nextItemWidth(float width) GEODE_EVENT_EXPORT_NORES(&nextItemWidth, (width));
    inline void indent() GEODE_EVENT_EXPORT_NORES(&indent, ());
    inline void unindent() GEODE_EVENT_EXPORT_NORES(&unindent, ());

    inline bool combo(
        geode::ZStringView label,
        int& current,
        std::span<char const*> items,
        int maxHeight = -1
    ) GEODE_EVENT_EXPORT_NORES(&combo, (label, current, items, maxHeight));

    template <UnderlyingIntegral T, typename R = std::initializer_list<geode::ZStringView>>
        requires
            std::ranges::range<R> &&
            std::same_as<std::remove_pointer_t<decltype(&*std::declval<R>().begin())> const, geode::ZStringView const>
    bool combo(geode::ZStringView label, T& current, R&& range, int maxHeight = -1) {
        return combo(
            label,
            reinterpret_cast<int&>(current),
            std::span(const_cast<geode::ZStringView*>(&*range.begin()), const_cast<geode::ZStringView*>(&*range.end())),
            maxHeight
        );
    }

    inline bool radio(geode::ZStringView label, int& current, int num) GEODE_EVENT_EXPORT_NORES(&radio, (label, current, num));

    template <UnderlyingIntegral T, UnderlyingIntegral U>
    bool radio(geode::ZStringView label, T& current, U value) {
        return radio(label, reinterpret_cast<int&>(current), reinterpret_cast<int&>(value));
    }

    inline void inputMultiline(geode::ZStringView label, std::string& text) GEODE_EVENT_EXPORT_NORES(&inputMultiline, (label, text));
}
