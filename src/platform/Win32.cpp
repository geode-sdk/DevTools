#include <Geode/modify/CCEGLView.hpp>
#include "platform.hpp"

using namespace cocos2d;

class $modify(CCEGLView) {
    void updateWindow(int width, int height) {
        shouldUpdateGDRenderBuffer() = true;
        CCEGLView::updateWindow(width, height);
    }
};