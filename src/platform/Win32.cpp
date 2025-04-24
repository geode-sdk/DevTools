#include <Geode/platform/platform.hpp>

#ifdef GEODE_IS_WINDOWS

#include <Geode/modify/CCEGLView.hpp>
#include "platform.hpp"
#include "../DevTools.hpp"

using namespace cocos2d;
using namespace geode;

ImGuiKey keyFromGLFW(int key) {
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
        return static_cast<ImGuiKey>(ImGuiKey_0 + (key - GLFW_KEY_0));
    } else if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        return static_cast<ImGuiKey>(ImGuiKey_A + (key - GLFW_KEY_A));
    }
    switch (key) {
        case GLFW_KEY_SPACE: return ImGuiKey_Space;
        case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
        case GLFW_KEY_COMMA: return ImGuiKey_Comma;
        case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
        case GLFW_KEY_UP: return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
        case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
        case GLFW_KEY_LEFT_SHIFT: case GLFW_KEY_RIGHT_SHIFT: return ImGuiKey_ModShift;
        case GLFW_KEY_LEFT_CONTROL: case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_ModCtrl;
        case GLFW_KEY_LEFT_ALT: case GLFW_KEY_RIGHT_ALT: return ImGuiKey_ModAlt;
        case GLFW_KEY_HOME: return ImGuiKey_Home;
        case GLFW_KEY_END: return ImGuiKey_End;
        case GLFW_KEY_DELETE: return ImGuiKey_Delete;
        case GLFW_KEY_ENTER: return ImGuiKey_Enter;
        // TODO: rest :-)
    }
    return ImGuiKey_None;
}

class $modify(CCEGLView) {
    void updateWindow(int width, int height) {
        shouldUpdateGDRenderBuffer() = true;
        CCEGLView::updateWindow(width, height);
    }

    void toggleFullScreen(bool value, bool borderless, bool fix) {
		if (!DevTools::get()->isSetup())
			return CCEGLView::toggleFullScreen(value, borderless, fix);

		DevTools::get()->destroy();
		CCEGLView::toggleFullScreen(value, borderless, fix);
		DevTools::get()->setup();
	}

    //todo: i dont care someone else can figure it out, it completely breaks keyboard support
    void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);
        }
        // in practice this is only used for arrow keys
        io.AddKeyEvent(keyFromGLFW(key), action != GLFW_RELEASE);
    }
};

#include "utils.hpp"

std::string formatAddressIntoOffsetImpl(uintptr_t addr, bool module) {
    HMODULE mod;

    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<char*>(addr), &mod)
    ) {
        mod = nullptr;
    }

    wchar_t buffer[MAX_PATH];
    std::string const module_name = (!mod || !GetModuleFileNameW(mod, buffer, MAX_PATH)) ? "Unknown" : std::filesystem::path(buffer).filename().string();

    if(module) return fmt::format("{} + {:#x}", module_name, addr - reinterpret_cast<uintptr_t>(mod));
    return fmt::format("{:#x}", addr - reinterpret_cast<uintptr_t>(mod));
}

#endif