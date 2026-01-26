//
// Created by jamie on 2025/5/26.
//

#ifndef UTILS_H
#define UTILS_H

#include "config.h"

#include <format>
#include <optional>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
class CommonUtils
{
public:
#if defined(ENABLE_SKYRIM_VR) || defined(ENABLE_SKYRIM_SE) || defined(ENABLE_SKYRIM_AE)
    #include "SKSE/Interfaces.h"

    static constexpr auto GetInterfaceFile(const std::string_view &fileName) -> std::string
    {
        const auto &pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
        return std::format("Data\\interface\\{}\\{}", pluginName, fileName);
    }
#endif

    static constexpr uint8_t HexCharToInt(char c)
    {
        if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
        if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
        return UINT8_MAX;
    }

    static constexpr auto HexToU32(std::string_view hex) -> std::optional<uint32_t>
    {
        if (hex.size() != 7 || !hex.starts_with("#")) return std::nullopt;

        auto hexSv1 = hex.substr(1);

        auto get_byte = [&](size_t idx, uint32_t &col) -> bool {
            auto p1 = HexCharToInt(hexSv1[idx * 2]);
            if (p1 == UINT8_MAX) return false;
            auto p2 = HexCharToInt(hexSv1[idx * 2 + 1]);
            if (p2 == UINT8_MAX) return false;
            col = p1 << 4 | p2;
            return true;
        };

        uint32_t r, g, b;
        if (get_byte(0, r) && get_byte(1, g) && get_byte(2, b))
        {
            return 0xFF << 24 | b << 16 | g << 8 | r;
        }
        return std::nullopt;
    }
};
}

#endif // UTILS_H
