# DevTools

Browser-like developer tools for Geode.

Press `F11` (`F10` for MacOS) to open up the dev tools.

## Features

 * Inspect the current node tree
 * Inspect individual nodes
 * Easily change attributes like the position and color of nodes
 * Multiple built-in themes

## API

Developers can register their own node's properties to show up in DevTools like so:

```cpp
#include <geode.devtools/include/API.hpp>

class MySprite : public CCSprite {
public:
    enum class RandomEnum {
        Option1,
        Option2,
        Option3
    };

    static void registerDevTools() {
        // The lambda provided will run every frame (given the node is selected)
        // for any node that is typeinfo_cast able into MySprite
        devtools::registerNode<MySprite>([](MySprite* node) {
            devtools::label("My Sprite");
            devtools::property("Some Flag", node->m_someFlag);
            devtools::property("Some Float", node->m_someFloat);
            devtools::property("Some Int", node->m_someInt);
            devtools::property("Some String", node->m_someString);
            devtools::property("Some Color", node->m_someColor);
            devtools::enumerable("Some Enum", node->m_someEnum, {
                { RandomEnum::Option1, "Option 1" },
                { RandomEnum::Option2, "Option 2" },
                { RandomEnum::Option3, "Option 3" }
            });
            devtools::button("Shake It", [&]{
                node->runAction(CCShaky3D::create(
                    0.5f, CCSize(10, 10), 5, false
                ));
            });
        });
    }

private:
    float m_someFloat = 3.14f;
    int m_someInt = 42;
    std::string m_someString = "text";
    ccColor3B m_someColor = {255, 0, 0};
    RandomEnum m_someEnum = RandomEnum::Option1;
    bool m_someFlag = true;
};

$on_mod(Loaded) {
    // makes sure DevTools is loaded before registering
    devtools::waitForDevTools([] {
        // use a static method on the class itself
        // so it can access private members
        MySprite::registerDevTools();
    });
}
```