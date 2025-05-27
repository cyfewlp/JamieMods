//
// Created by jamie on 2025/5/26.
//

#ifndef UTILS_H
#define UTILS_H

#include "SKSE/Interfaces.h"
#include "common/config.h"

#include <format>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
class CommonUtils
{
public:
    static constexpr auto GetInterfaceFile(const std::string_view &fileName) -> std::string
    {
        const auto &pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
        return std::format("Data\\interface\\{}\\{}", pluginName, fileName);
    }
};
}

#endif // UTILS_H
