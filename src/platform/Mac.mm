#include <Geode/platform/platform.hpp>

#ifdef GEODE_IS_MACOS

#include "utils.hpp"

#include <Geode/utils/string.hpp>
#include <array>
#include <thread>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <algorithm>
#include <fmt/core.h>

#include <mach-o/dyld_images.h>
#include <mach-o/dyld.h>
#import <Foundation/Foundation.h>

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
    return std::filesystem::path(imageName).filename().string();
}

std::string formatAddressIntoOffsetImpl(uintptr_t addr, bool module) {
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

    if(module) return fmt::format("{} + {:#x}", imageName, addr - base);
    else return fmt::format("{:#x}", addr - base);
}

#endif