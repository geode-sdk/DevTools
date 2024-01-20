#include <Geode/platform/platform.hpp>

#ifdef GEODE_IS_ANDROID

using namespace geode::prelude;

#include "utils.hpp"

std::string formatAddressIntoOffsetImpl(uintptr_t addr) {
    return fmt::format("idk + {:#x}", addr);
}

#endif