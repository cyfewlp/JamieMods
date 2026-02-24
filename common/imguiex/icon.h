//
// Created by jamie on 2026/2/23.
//

#pragma once

#include <string_view>
#include <utility>

namespace ImGuiEx
{
template <char32_t Codepoint>
struct Icon
{
private:
    // UTF-8 encoding uses up to 4 bytes + null terminator
    static constexpr auto Encode() -> std::pair<std::array<char, 4>, size_t>
    {
        std::array<char, 4> buf{};
        size_t              len = 0;
        if constexpr (Codepoint <= 0x7F)
        {
            buf[0] = static_cast<char>(Codepoint);
            len    = 1;
        }
        else if constexpr (Codepoint <= 0x7FF)
        {
            buf[0] = static_cast<char>(0xC0 | (Codepoint >> 6));
            buf[1] = static_cast<char>(0x80 | (Codepoint & 0x3F));
            len    = 2;
        }
        else if constexpr (Codepoint <= 0xFFFF)
        {
            buf[0] = static_cast<char>(0xE0 | (Codepoint >> 12));
            buf[1] = static_cast<char>(0x80 | ((Codepoint >> 6) & 0x3F));
            buf[2] = static_cast<char>(0x80 | (Codepoint & 0x3F));
            len    = 3;
        }
        else
        {
            buf[0] = static_cast<char>(0xF0 | (Codepoint >> 18));
            buf[1] = static_cast<char>(0x80 | ((Codepoint >> 12) & 0x3F));
            buf[2] = static_cast<char>(0x80 | ((Codepoint >> 6) & 0x3F));
            buf[3] = static_cast<char>(0x80 | (Codepoint & 0x3F));
            len    = 4;
        }
        return {buf, len};
    }

    static constexpr auto encoded = Encode();

public:
    static constexpr auto data = encoded.first;
    static constexpr auto size = encoded.second;

    constexpr operator std::string_view() const // NOLINT(*-explicit-constructor)
    {
        return {data.data(), size};
    }
};

template <char32_t Codepoint>
inline constexpr Icon<Codepoint> icon{};

} // namespace ImGuiEx
