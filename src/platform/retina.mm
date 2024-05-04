#include <Geode/platform/platform.hpp>

#ifdef GEODE_IS_MACOS

#include "../DevTools.hpp"

#include <OpenGL/gl.h>
#define CommentType CommentTypeDummy
#import "Cocoa/Cocoa.h"
#undef CommentType

float DevTools::retinaFactor() {
    float displayScale = 1.f;
    if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)]) {
        NSArray* screens = [NSScreen screens];
        for (int i = 0; i < screens.count; i++) {
            float s = [screens[i] backingScaleFactor];
            if (s > displayScale)
                displayScale = s;
        }
    }
    return displayScale;
}

#endif
