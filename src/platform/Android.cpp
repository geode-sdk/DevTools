#include <Geode/platform/platform.hpp>

#ifdef GEODE_IS_ANDROID

using namespace geode::prelude;

#include "utils.hpp"

std::string formatAddressIntoOffsetImpl(uintptr_t addr, bool module) {
    if (addr > base::get() && addr - 0x1000000 < base::get()) {
        if(module) return fmt::format("libcocos2d.so + {:#x}", addr - base::get());
        else return fmt::format("{:#x}", addr - base::get());
    }
    return fmt::format("{:#x}", addr);
}

#endif