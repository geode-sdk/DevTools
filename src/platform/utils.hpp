#pragma once

#include <string>
#include <stdint.h>

std::string formatAddressIntoOffset(uintptr_t addr);

std::string formatAddressIntoOffsetImpl(uintptr_t addr);
