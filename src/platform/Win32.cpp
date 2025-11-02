#include <Geode/platform/platform.hpp>

#ifdef GEODE_IS_WINDOWS

#include <Geode/modify/CCEGLView.hpp>
#include "platform.hpp"
#include "../DevTools.hpp"

using namespace cocos2d;
using namespace geode;

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

// mostly copied from gd-imgui-cocos
void setMouseCursor() {
    // Shows imgui's cursor instead of hidden cursor if out of GD Window
    bool isCursorVisible = false;
    CURSORINFO ci = { sizeof(ci) };
    if (GetCursorInfo(&ci)) {
        isCursorVisible = (ci.flags & CURSOR_SHOWING) != 0;
    }
    // whether to draw a fake cursor
    ImGui::GetIO().MouseDrawCursor = DevTools::get()->isVisible() && !isCursorVisible && !shouldPassEventsToGDButTransformed();

    struct GLFWCursorData {
        void* next = nullptr;
        HCURSOR cursor;
    };
    auto& cursorField = *reinterpret_cast<GLFWCursorData**>(reinterpret_cast<uintptr_t>(
        CCEGLView::get()->getWindow()) + 0x50);

    auto cursor = ImGui::GetIO().MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    static ImGuiMouseCursor lastCursor = ImGuiMouseCursor_COUNT;
    if (cursor != lastCursor) {
        lastCursor = cursor;

        auto winCursor = IDC_ARROW;
        switch (cursor) {
            case ImGuiMouseCursor_Arrow: winCursor = IDC_ARROW; break;
            case ImGuiMouseCursor_TextInput: winCursor = IDC_IBEAM; break;
            case ImGuiMouseCursor_ResizeAll: winCursor = IDC_SIZEALL; break;
            case ImGuiMouseCursor_ResizeEW: winCursor = IDC_SIZEWE; break;
            case ImGuiMouseCursor_ResizeNS: winCursor = IDC_SIZENS; break;
            case ImGuiMouseCursor_ResizeNESW: winCursor = IDC_SIZENESW; break;
            case ImGuiMouseCursor_ResizeNWSE: winCursor = IDC_SIZENWSE; break;
            case ImGuiMouseCursor_Hand: winCursor = IDC_HAND; break;
            case ImGuiMouseCursor_NotAllowed: winCursor = IDC_NO; break;
        }
        if (cursorField) {
            cursorField->cursor = LoadCursor(NULL, winCursor);
        }
        else {
            // must be heap allocated
            cursorField = new GLFWCursorData {
                .next = nullptr,
                .cursor = LoadCursor(NULL, winCursor)
            };
        }
    }
}

#endif

