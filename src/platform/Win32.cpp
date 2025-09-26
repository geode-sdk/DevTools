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

#endif