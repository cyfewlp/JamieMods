#pragma once

#define TOSTRING(x) TOSTRING_(x)
#define TOSTRING_(x) #x

namespace AddressLibTool {
    bool InitializeOffsets();
    bool DumpSpecialVerAddress(unsigned int major, unsigned int minor);
}