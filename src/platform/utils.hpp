#pragma once

#include <string>
#include <stdint.h>

std::string formatAddressIntoOffset(uintptr_t addr, bool module);

std::string formatAddressIntoOffsetImpl(uintptr_t addr, bool module);
