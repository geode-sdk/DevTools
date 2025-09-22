#include <Geode/platform/platform.hpp>

#if defined(GEODE_IS_MACOS) || defined(GEODE_IS_IOS)

#include "utils.hpp"

#include <Geode/utils/string.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/loader/Log.hpp>
#include <array>
#include <thread>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <algorithm>
#include <fmt/core.h>
#include <filesystem>

#include <mach-o/dyld_images.h>
#include <mach-o/dyld.h>
#import <Foundation/Foundation.h>

#import <CoreGraphics/CoreGraphics.h>
#ifdef GEODE_IS_MACOS
#include <ImageIO/CGImageDestination.h>
#else 
#import <UIKit/UIKit.h>
#endif

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

void saveRenderToFile(std::vector<uint8_t> const& data, int width, int height, char const* filename) {
    assert(width * height * 4 == data.size());

    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, &data[0], data.size(), NULL);
    if (!provider) {
        geode::log::error("Failed to create CGDataProvider");
        return;
    }

    // create cgImg
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    if (!colorSpace) {
        geode::log::error("Failed to create CGColorSpace");
        CGDataProviderRelease(provider);
        return;
    }
    CGImageRef cgImg = CGImageCreate(
        width,
        height,
        8,
        32,
        width * 4,
        colorSpace,
        kCGImageAlphaPremultipliedLast,
        provider,
        NULL,
        false,
        kCGRenderingIntentDefault
    );

    if (!cgImg) {
        geode::log::error("Failed to create CGImage");
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpace);
        return;
    }

    #ifdef GEODE_IS_IOS
    UIImage* image = [UIImage imageWithCGImage:cgImg];
    if (!image) {
        geode::log::error("Failed to create UIImage");
        CGImageRelease(cgImg);
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpace);
        return;
    }

    NSData* pngData = UIImagePNGRepresentation(image);
    if (!pngData) {
        geode::log::error("Failed to create PNG data from UIImage");
        CGImageRelease(cgImg);
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpace);
        return;
    }

    if (auto err = geode::utils::file::writeBinary(filename, { (uint8_t*)pngData.bytes, (uint8_t*)pngData.bytes + pngData.length }).err()) {
        geode::log::error("Failed to write image to {}: {}", filename, err);
    }
    #else
    CFMutableDataRef pngData = CFDataCreateMutable(NULL, 0);
    CGImageDestinationRef destination = CGImageDestinationCreateWithData(pngData, kUTTypePNG, 1, NULL);
    if (!destination) {
        geode::log::error("Failed to create CGImageDestination");
        CFRelease(pngData);
        CGImageRelease(cgImg);
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpace);
        return;
    }

    CGImageDestinationAddImage(destination, cgImg, nil);
    if (!CGImageDestinationFinalize(destination)) {
        geode::log::error("Failed to write image to data");
        CFRelease(destination);
        CFRelease(pngData);
        CGImageRelease(cgImg);
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpace);
        return;
    }

    std::vector<uint8_t> vec;
    vec.resize(CFDataGetLength(pngData));
    CFDataGetBytes(pngData, CFRangeMake(0, vec.size()), vec.data());

    if (auto err = geode::utils::file::writeBinary(filename, vec).err()) {
        geode::log::error("Failed to write image to {}: {}", filename, err);
    }

    CFRelease(destination);
    CFRelease(pngData);
    #endif
    CGImageRelease(cgImg);
    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);
}


#endif