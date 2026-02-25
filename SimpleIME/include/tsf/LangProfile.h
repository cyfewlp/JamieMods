//
// Created by jamie on 2026/2/6.
//

#pragma once

#include <string>
#include <winnt.h>

template <>
struct std::hash<GUID>
{
    auto operator()(const GUID &guid) const noexcept -> std::size_t
    {
        std::size_t const h1            = std::hash<uint32_t>()(guid.Data1);
        std::size_t const h2            = std::hash<uint16_t>()(guid.Data2);
        std::size_t const h3            = std::hash<uint16_t>()(guid.Data3);
        uint64_t          data4Combined = 0;
        std::memcpy(&data4Combined, guid.Data4, sizeof(data4Combined));
        std::size_t const h4 = std::hash<uint64_t>()(data4Combined);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

namespace Ime
{
constexpr auto LANGID_ENG = 0x409;

struct LangProfile
{
    std::string desc{};
    CLSID       clsid{};
    GUID        guidProfile{};
    LANGID      langid{};
};

const auto DEFAULT_LANG_PROFILE = LangProfile{"ENG", CLSID_NULL, GUID_NULL, LANGID_ENG};

inline auto ToStringFromGUID2(const GUID &guid, std::wstring &wsGuid) -> void
{
    wsGuid.resize(40);
    StringFromGUID2(guid, wsGuid.data(), static_cast<int>(wsGuid.size()));
}

} // namespace Ime
