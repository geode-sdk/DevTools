#include <Geode/platform/platform.hpp>

#ifdef GEODE_IS_MACOS

#include "utils.hpp"

#include <Geode/loader/Log.hpp>
#include <Geode/utils/string.hpp>
#include <array>
#include <thread>
#include <array>
#include <ghc/fs_fwd.hpp>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <sstream>
#include <algorithm>
#include <fmt/core.h>

#include <mach-o/dyld_images.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
#import <Foundation/Foundation.h>

using namespace geode::prelude;

static std::vector<struct dyld_image_info const*> getAllImages() {
    std::vector<struct dyld_image_info const*> images;
    struct task_dyld_info dyldInfo;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)&dyldInfo, &count) == KERN_SUCCESS) {
        struct dyld_all_image_infos* imageInfos = (struct dyld_all_image_infos*)dyldInfo.all_image_info_addr;

        for (size_t i = 0; i < imageInfos->infoArrayCount; ++i) {
            images.push_back(&imageInfos->infoArray[i]);
        }
    }

    return images;
}

static struct dyld_image_info const* imageFromAddress(void const* addr) {
    if (addr == nullptr) {
        return nullptr;
    }

    auto loadedImages = getAllImages();
    std::sort(loadedImages.begin(), loadedImages.end(), [](auto const a, auto const b) {
        return (uintptr_t)a->imageLoadAddress < (uintptr_t)b->imageLoadAddress;
    });
    auto iter = std::upper_bound(loadedImages.begin(), loadedImages.end(), addr, [](auto const addr, auto const image) {
        return (uintptr_t)addr < (uintptr_t)image->imageLoadAddress;
    });

    if (iter == loadedImages.begin()) {
        return nullptr;
    }
    --iter;

    auto image = *iter;
    auto imageAddress = (uintptr_t)image->imageLoadAddress;
    if ((uintptr_t)addr >= imageAddress/* && (uintptr_t)addr < imageAddress + imageSize*/) {
        return image;
    }
    return nullptr;
}

static std::string getImageName(struct dyld_image_info const* image) {
    if (image == nullptr) {
        return "Unknown";
    }
    std::string imageName = image->imageFilePath;
    if (imageName.empty()) {
        return "Unknown";
    }
    return ghc::filesystem::path(imageName).filename().string();
}

std::string formatAddressIntoOffsetImpl(uintptr_t addr) {
    auto image = imageFromAddress(reinterpret_cast<void const*>(addr));
    std::string imageName;
    uintptr_t base;
    if (image == nullptr) {
        base = 0;
        imageName = "Unknown";
    } else {
        base = (uintptr_t)image->imageLoadAddress;
        imageName = getImageName(image);
    }

    return fmt::format("{} + {:#x}", imageName, addr - base);
}

uint32_t getProtection(void const* addr) {
    kern_return_t status;
    mach_vm_size_t vmsize;
    mach_vm_address_t vmaddress = reinterpret_cast<mach_vm_address_t>(addr);
    vm_region_basic_info_data_t info;
    mach_msg_type_number_t infoCount = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object;

    status = mach_vm_region(
        mach_task_self(),
        &vmaddress,
        &vmsize,
        VM_REGION_BASIC_INFO_64,
        reinterpret_cast<vm_region_info_t>(&info),
        &infoCount,
        &object
    );

    if (status != KERN_SUCCESS) {
        return 0;
    }

    return info.protection;
}

bool canReadAddress(void* ptr) {
    vm_map_t task = mach_task_self();
    mach_vm_address_t address = (mach_vm_address_t)ptr;
    mach_vm_size_t size = 0;
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object_name;
    kern_return_t ret = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &count, &object_name);
    if (ret != KERN_SUCCESS) return false;
    return ((mach_vm_address_t)ptr) >= address && ((info.protection & VM_PROT_READ) == VM_PROT_READ);
}

char const* rttiName(char const**** addr) {
    if (!canReadAddress(addr)) return nullptr;
    // log::debug("addr {}", (void*)addr);

    auto typeinfo = *addr - 1;
    if (!canReadAddress(typeinfo)) return nullptr;
    // log::debug("typeinfo {}", (void*)typeinfo);

    auto name = *typeinfo + 1;
    if (!canReadAddress(name)) return nullptr;
    // log::debug("name {}", (void*)name);

    auto ret = *name;
    if (!canReadAddress((void*)ret)) return nullptr;
    // log::debug("ret {}", (void*)ret);

    return ret;
}

std::string formatStructRTTIs(uintptr_t addr, uintptr_t start, uintptr_t end) {
    std::stringstream out;
    auto check = reinterpret_cast<char const*****>(addr);
    for (auto offset = start; offset < end; offset += 0x8) {
        // log::debug("offset: {}", offset);
        auto name = rttiName(check[offset / 8]);

        if (name) {
            out << reinterpret_cast<void*>(offset) << ": " << name << " (" << (void*)check[offset / 8] << ")\n";
        }
    }
    return out.str();
}

#endif