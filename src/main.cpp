#include "versionlibdb.h"
// #include "versiondb.h"
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

void* MyAddress = NULL;
unsigned long long MyOffset = 0;

bool InitializeOffsets()
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

bool DumpSpecificVersion()
{
    VersionDb db;

    // Try to load database of version 1.5.62.0 regardless of running executable version.
    if (!db.Load(1, 6, 353, 0)) {
        spdlog::error("Failed to load database for 1.6.353.0!");
        return false;
    }

    // Write out a file called offsets-1.5.62.0.txt where each line is the ID and offset.
    db.Dump("offsets-1.6.353.txt");
    spdlog::info("Dumped offsets for 1.6.353.0");
    return true;
}

template <typename T>
 void exampleFunction(const T& container)
{
    // 使用 typename 关键字声明迭代器类型
    typename T::const_iterator it;

    for (it = container.begin(); it != container.end(); ++it) {
        // 处理容器中的元素
    }
}

int main() { DumpSpecificVersion(); }