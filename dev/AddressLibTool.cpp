#include "AddressLibTool.hpp"
#include "versionlibdb.h"
#include <fmt/format.h>
#include <iostream>
#include <spdlog/spdlog.h>

// #include "versiondb.h"

void* MyAddress = NULL;
unsigned long long MyOffset = 0;

bool
AddressLibTool::InitializeOffsets()
{
    // Allocate on stack so it will be unloaded when we exit this function.
    // No need to have the whole database loaded and using up memory for no reason.
    VersionDb db;

    // Load database with current executable version.
    if (!db.Load()) {
        spdlog::error("Failed to load version database for current executable!");
        return false;
    } else {
        // "SkyrimSE.exe", "1.5.97.0"
        spdlog::info(
          "Loaded database for %s version %s.", db.GetModuleName().c_str(), db.GetLoadedVersionString().c_str());
    }

    // This address already includes the base address of module so we can use the address directly.
    MyAddress = db.FindAddressById(123);
    if (MyAddress == NULL) {
        spdlog::error("Failed to find address!");
        return false;
    }

    // This offset does not include base address. Actual address would be ModuleBase + MyOffset.
    if (!db.FindOffsetById(123, MyOffset)) {
        spdlog::error("Failed to find offset for my thing!");
        return false;
    }

    // Everything was successful.
    return true;
}

bool
AddressLibTool::DumpSpecialVerAddress(unsigned int major, unsigned int minor)
{
    VersionDb db;
    // Try to load database of version 1.5.62.0 regardless of running executable version.
    std::string s = fmt::format("1.{}.{}.0", major, minor);
    if (!db.Load(1, major, minor, 0)) {
        spdlog::error("Failed to load database for {}!", s.c_str());
        return false;
    }

    // Write out a file called offsets-1.5.62.0.txt where each line is the ID and offset.
    db.Dump(fmt::format("offset-{}.txt", s.c_str()).c_str());
    spdlog::info("Dumped offsets for {}", s.c_str());
    return true;
}