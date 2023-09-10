#pragma once

#include <string>
#include <stdint.h>

std::string formatAddressIntoOffset(uintptr_t addr);

std::string formatAddressIntoOffsetImpl(uintptr_t addr);

std::string formatStructRTTIs(uintptr_t addr, uintptr_t start, uintptr_t end);