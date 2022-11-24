#pragma once

// C++20 disallows const u8char_t* -> const char* despite imgui  working with that
#define U8STR(str) reinterpret_cast<const char*>(str)

