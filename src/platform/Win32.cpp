#include <Geode/platform/platform.hpp>

#ifdef GEODE_IS_WINDOWS

#include <Geode/modify/CCEGLView.hpp>
#include "platform.hpp"

using namespace cocos2d;

class $modify(CCEGLView) {
    void updateWindow(int width, int height) {
        shouldUpdateGDRenderBuffer() = true;
        CCEGLView::updateWindow(width, height);
    }
};

#endif