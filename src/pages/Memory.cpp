#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include "../ImGui.hpp"
#include <chrono>
#include <algorithm>

using namespace geode::prelude;

#if defined(GEODE_IS_WINDOWS)

#include <Windows.h>

bool canReadAddr(uintptr_t addr, size_t size) {
    return !IsBadReadPtr(reinterpret_cast<void*>(addr), size);
}

#elif defined(GEODE_IS_ANDROID)

auto getReadableAddresses() {
    using namespace std::chrono_literals;
    static std::vector<std::pair<uintptr_t, uintptr_t>> cache;
    // static auto lastCheck = std::chrono::high_resolution_clock::now();
    // auto now = std::chrono::high_resolution_clock::now();
    if (cache.empty()) {
        cache.clear();
        std::ifstream mappings("/proc/self/maps");
        std::string line;
        while (std::getline(mappings, line)) {
            uintptr_t start, end;
            char flags[4];
            std::sscanf(line.c_str(), "%" PRIxPTR "-%" PRIxPTR " %4c", &start, &end, flags);
            if (flags[0] == 'r') {
                cache.push_back({ start, end });
            }
        }
    }
    return cache;
} 

bool canReadAddr(uintptr_t addr, size_t size) {
    if (addr <= 0x10000) return false;
#ifdef GEODE_IS_ANDROID64
    // if ((addr & 0xFF00000000000000) == 0) return false;
    addr = addr & ~(0xFF00000000000000);
#endif
    auto const& mappings = getReadableAddresses();
    auto value = std::make_pair(addr, addr + size);
    // get the largest start which is <= addr
    auto it = std::upper_bound(mappings.rbegin(), mappings.rend(), value, [](auto const& a, auto const& b) {
        return a.first >= b.first;
    });
    if (it == mappings.rend()) return false;
    // it->first is already known to be <= addr,
    // now just check the end
    return value.second < it->second;
}

#endif

struct SafePtr {
    uintptr_t addr = 0;
    SafePtr(uintptr_t addr) : addr(addr) {}
    SafePtr(const void* addr) : addr(reinterpret_cast<uintptr_t>(addr)) {}

    bool operator==(void* ptr) const { return as_ptr() == ptr; }
    // breaks clang
    // operator bool() const { return addr != 0; }

    void* as_ptr() const { return reinterpret_cast<void*>(addr); }

    bool is_safe(int size) {
        return canReadAddr(addr, size);
    }

    bool read_into(void* buffer, int size) {
        if (!is_safe(size)) return false;

        std::memcpy(buffer, as_ptr(), size);
        return true;
    }

    template <class T>
    T read() {
        T result{};
        read_into(&result, sizeof(result));
        return result;
    }

    SafePtr read_ptr() {
        return SafePtr(this->read<uintptr_t>());
    }

    SafePtr operator+(intptr_t offset) const {
        return SafePtr(addr + offset);
    }
    SafePtr operator-(intptr_t offset) const {
        return SafePtr(addr - offset);
    }

    std::string_view read_c_str(int max_size = 512) {
        if (!is_safe(max_size)) return "";
        auto* c_str = reinterpret_cast<const char*>(as_ptr());
        for (int i = 0; i < max_size; ++i) {
            if (c_str[i] == 0)
                return std::string_view(c_str, i);
        }
        return "";
    }
};

std::string_view demangle(std::string_view mangled) {
    static std::unordered_map<std::string_view, std::string> cached;
    auto it = cached.find(mangled);
    if (it != cached.end()) {
        return it->second;
    }
    auto parts = utils::string::split(std::string(mangled.substr(4)), "@");
    std::string result;
    for (const auto& part : utils::ranges::reverse(parts)) {
        if (part.empty()) continue;
        if (!result.empty())
            result += "::";
        result += part;
    }
    return cached[mangled] = result;
}

struct RttiInfo {
    SafePtr ptr;
    RttiInfo(SafePtr ptr) : ptr(ptr) {}

    std::optional<std::string_view> class_name() {
        // TODO: maybe cache from the typeinfo pointer?
    #if defined(GEODE_IS_WINDOWS)
        auto vtable = ptr.read_ptr();
        if (!vtable.addr) return {};
        auto rttiObj = (vtable - 4).read_ptr();
        if (!rttiObj.addr) return {};
        // always 0
        auto signature = rttiObj.read<int>();
        if (signature != 0) return {};
        auto rttiDescriptor = (rttiObj + 12).read_ptr();
        if (!rttiDescriptor.addr) return {};
        return demangle((rttiDescriptor + 8).read_c_str());
    #else
        auto vtable = ptr.read_ptr();
        if (!vtable.addr) return {};
        auto typeinfo = (vtable - sizeof(void*)).read_ptr();
        if (!typeinfo.addr) return {};
        auto typeinfoName = (typeinfo + sizeof(void*)).read_ptr();
        if (!typeinfoName.addr) return {};
        return typeinfoName.read_c_str();
    #endif
    }
};

std::optional<std::string_view> findStdString(SafePtr ptr) {
#if defined(GEODE_IS_WINDOWS)
    // scan for std::string (msvc)
    auto size = (ptr + 16).read<size_t>();
    auto capacity = (ptr + 20).read<size_t>();
    if (size > capacity || capacity < 15) return {};
    // dont care about ridiculous sizes (> 100mb)
    if (capacity > 1e8) return {};
    char* data = nullptr;
    if (capacity == 15) {
        data = reinterpret_cast<char*>(ptr.as_ptr());
    } else {
        data = reinterpret_cast<char*>(ptr.read_ptr().as_ptr());
    }
#elif defined(GEODE_IS_ANDROID)
    auto internalData = ptr.read_ptr();
    if (!internalData.addr) return {};
    auto size = (internalData - (3 * sizeof(void*))).read<size_t>();
    auto capacity = (internalData - (2 * sizeof(void*))).read<size_t>();
    auto refCount = (internalData - (1 * sizeof(void*))).read<int>();
    if (size > capacity || refCount < 0) return {};
    if (capacity > 1e8) return {};
    char* data = reinterpret_cast<char*>(internalData.as_ptr());
#endif
    if (data == nullptr || !SafePtr(data).is_safe(capacity)) return {};
    // quick null term check
    if (data[size] != 0) return {};
    if (strlen(data) != size) return {};
    return std::string_view(data, size);
}

void DevTools::drawMemory() {
    using namespace std::chrono_literals;
    static auto lastRender = std::chrono::high_resolution_clock::now();

    static char buffer[256] = {'0', '\0'};
    bool changed = ImGui::InputText("Addr", buffer, sizeof(buffer));
    static int size = 0x100;
    changed |= ImGui::DragInt("Size", &size, 16.f, 0, 0, "%x");
    if (size < 4) {
        size = 4;
    }

    if (ImGui::Button("Selected Node")) {
        auto str = fmt::format("{}", fmt::ptr(m_selectedNode.data()));
        std::memcpy(buffer, str.c_str(), str.size() + 1);
        changed = true;
    }

    auto const timeNow = std::chrono::high_resolution_clock::now();

    // if (timeNow - lastRender < 0.5s) return;
    // lastRender = timeNow;

    uintptr_t addr = 0;
    try {
        addr = std::stoull(buffer, nullptr, 16);
    } catch (...) {}

    static std::vector<std::string> texts;
    if (changed) {
        texts.clear();
        for (int offset = 0; offset < size; offset += sizeof(void*)) {
            SafePtr ptr = addr + offset;
            RttiInfo info(ptr.read_ptr());
            auto name = info.class_name();
            if (name) {
                texts.push_back(fmt::format("[{:04x}] {}", offset, *name));
            } else {
                auto maybeStr = findStdString(ptr);
                if (maybeStr) {
                    auto str = maybeStr->substr(0, 30);
                    // escapes new lines and stuff for me :3
                    auto fmted = matjson::Value(std::string(str)).dump(0);
                    texts.push_back(fmt::format("[{:04x}] maybe std::string {}, {}", offset, maybeStr->size(), fmted));
                }
            }
        }
    }

    ImGui::PushFont(m_monoFont);
    for (const auto& text : texts) {
        ImGui::TextUnformatted(text.data(), text.data() + text.size());
        // ImGui::Text("%s", text);
    }
    ImGui::PopFont();
}
