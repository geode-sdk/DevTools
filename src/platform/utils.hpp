#pragma once

#include <string>
#include <stdint.h>
#include <Geode/cocos/cocoa/CCObject.h>
#ifndef GEODE_IS_WINDOWS
#include <cxxabi.h>
#endif

static inline std::string getNodeName(cocos2d::CCObject* node) {
#ifdef GEODE_IS_WINDOWS
    return typeid(*node).name() + 6;
#else 
    std::string ret;

    int status = 0;
    auto demangle = abi::__cxa_demangle(typeid(*node).name(), 0, 0, &status);
    if (status == 0) {
        ret = demangle;
    }
    free(demangle);

    return ret;
#endif
}

std::string formatAddressIntoOffset(uintptr_t addr, bool module);

std::string formatAddressIntoOffsetImpl(uintptr_t addr, bool module);
