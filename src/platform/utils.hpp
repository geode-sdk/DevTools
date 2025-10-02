#pragma once

#include <string>
#include <stdint.h>
#include <Geode/cocos/cocoa/CCObject.h>

std::string formatAddressIntoOffset(uintptr_t addr, bool module);

std::string formatAddressIntoOffsetImpl(uintptr_t addr, bool module);

std::vector<uint8_t> renderToBytes(cocos2d::CCNode* node, int& width, int& height);
void saveRenderToFile(std::vector<uint8_t> const& data, int width, int height, char const* filename);
