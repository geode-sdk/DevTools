#include "utils.hpp"

#include <unordered_map>

std::string formatAddressIntoOffset(uintptr_t addr) {
    static std::unordered_map<uintptr_t, std::string> formatted;
    auto it = formatted.find(addr);
    if (it != formatted.end()) {
        return it->second;
    } else {
        auto const txt = formatAddressIntoOffsetImpl(addr);
        formatted.insert({ addr, txt });
        return txt;
    }
}