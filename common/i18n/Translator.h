//
// Created by jamie on 2026/1/28.
//

#pragma once

#if defined(_DEBUG) && !defined(RECORD_KEY)
    #define RECORD_KEY
#endif

#include "common/log.h"

#include <cstdint>
#include <string>
#include <unordered_map>

#ifdef RECORD_KEY
    #include <unordered_set>
#endif

namespace i18n
{
class Translator
{
public:
    using LanguageMap = std::unordered_map<uint64_t, std::string>;

    Translator() {}

    explicit Translator(const LanguageMap &map) : m_currentMap(map) {}

    explicit Translator(LanguageMap &&map) : m_currentMap(std::move(map)) {}

    explicit Translator(Translator &&other) noexcept
    {
        if (this != &other)
        {
            m_currentMap = std::move(other.m_currentMap);
        }
    }

    Translator &operator=(Translator &&other) noexcept
    {
        if (this != &other)
        {
            m_currentMap = std::move(other.m_currentMap);
        }
        return *this;
    }

    Translator &operator=(const Translator &other) = delete;

    auto Translate(const uint64_t hash, const std::string_view &defaultText) const -> std::string_view
    {
        if (const auto it = m_currentMap.find(hash); it != m_currentMap.end())
        {
            return it->second;
        }
        return defaultText;
    }

    // template <typename... Args>
    // auto Format(uint64_t hash, std::string_view defaultFmt, Args &&...args) const -> std::string
    // {
    //     auto fmt = Translate(hash, defaultFmt);
    //     return std::format(fmt, std::make_format_args(args...));
    // }

private:
    LanguageMap m_currentMap;
};

#ifdef RECORD_KEY

/**
 * Non-crypto hashes in C++: FNV 1/1a
 * https://asecuritysite.com/encryption/smh_fnv
 */
constexpr auto HashKey(const std::string_view str) -> uint64_t
{
    static std::unordered_set<std::string> hashes;
    hashes.emplace(str);
    // just store it.We can add a breakpoint in this line to check used keys;
#else
constexpr auto HashKey(const std::string_view str) -> uint64_t
{
#endif
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (char c : str)
    {
        hash ^= static_cast<uint64_t>(c);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

consteval auto operator""_hash(const char *str, size_t length) -> uint64_t
{
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (int i = 0; i < length; i++)
    {
        hash ^= static_cast<uint64_t>(str[i]);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

} // namespace i18n
