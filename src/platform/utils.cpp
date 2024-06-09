#include "utils.hpp"

#include <unordered_map>

std::string formatAddressIntoOffset(uintptr_t addr, bool module) {
    static std::unordered_map<uintptr_t, std::pair<std::string, std::string>> formatted;
    auto it = formatted.find(addr);
    if (it != formatted.end()) {
        if(module) return it->second.first;
        else return it->second.second;
    } else {
        auto const txt = formatAddressIntoOffsetImpl(addr, true);
        auto const txtNoModule = formatAddressIntoOffsetImpl(addr, false);
        auto const pair = std::make_pair(txt, txtNoModule);
        formatted.insert({ addr, pair });
        if(module) return pair.first;
        else return pair.second;
    }
}