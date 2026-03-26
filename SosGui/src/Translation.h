//
// Created by jamie on 2025/4/7.
//

#pragma once

namespace SosGui
{

namespace Translation
{
void Translate(const char *key, std::string &result);

auto Translate(const char *key) -> std::string;

auto TranslateNoCache(const char *key) -> std::string;

constexpr auto Translate(const std::string &key) -> std::string
{
    return Translate(key.c_str());
}

template <typename... Args>
auto Translate(const char *key, Args &&...args) -> std::string
{
    std::string templateStr;
    Translate(key, templateStr);
    return std::vformat(templateStr, std::make_format_args(std::forward<Args>(args)...));
}

auto TranslateIgnoreNested(const std::string &a_key, std::string &a_result) -> bool;

auto TranslateIgnoreNested(const std::string &a_key) -> std::string;

template <typename... Args>
auto Translate(const char *key, bool ignoreNested, Args &&...args) -> std::string
{
    std::string templateStr;
    if (ignoreNested)
    {
        TranslateIgnoreNested(key, templateStr);
    }
    else
    {
        Translate(key, templateStr);
    }
    return std::vformat(templateStr, std::make_format_args(args...));
}
} // namespace Translation

[[nodiscard]] constexpr auto operator""_T(const char *_Str, size_t _Len) noexcept -> std::string
{
    return Translation::Translate(std::string_view(_Str, _Len).data());
}
} // namespace SosGui
