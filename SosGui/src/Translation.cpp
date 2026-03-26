//
// Created by jamie on 2025/4/7.
//

#include "Translation.h"

#include <common/log.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Translation
{
void Translate(const char *key, std::string &result)
{
    static std::unordered_map<std::string, std::string> cache;
    if (cache.contains(key))
    {
        result = cache.at(key);
        return;
    }
    result.assign(key);
    SKSE::Translation::Translate(key, result);
    cache.emplace(key, result);
}

auto Translate(const char *key) -> std::string
{
    std::string result;
    Translate(key, result);
    return result;
}

auto TranslateNoCache(const char *key) -> std::string
{
    std::string result(key);
    SKSE::Translation::Translate(key, result);
    return result;
}

auto TranslateIgnoreNested(const std::string &a_key, std::string &a_result) -> bool
{
    if (!a_key.starts_with('$'))
    {
        return false;
    }

    const auto scaleformManager = RE::BSScaleformManager::GetSingleton();
    const auto loader           = scaleformManager ? scaleformManager->loader : nullptr;
    const auto translator       = loader ? loader->GetStateAddRef<RE::GFxTranslator>(RE::GFxState::StateType::kTranslator) : nullptr;

    if (!translator)
    {
        log_warn("Failed to get Scaleform translator");
        return false;
    }

    // Count braces to find what to replace
    std::string key = a_key;

    // Lookup translation
    std::wstring         key_utf16 = SKSE::stl::utf8_to_utf16(key).value_or(L""s);
    RE::GFxWStringBuffer result;

    RE::GFxTranslator::TranslateInfo translateInfo;
    translateInfo.key    = key_utf16.c_str();
    translateInfo.result = std::addressof(result);

    translator->Translate(std::addressof(translateInfo));
    if (result.empty())
    {
        return false;
    }

    a_result = SKSE::stl::utf16_to_utf8(result.c_str()).value_or(""s);
    return true;
}

auto TranslateIgnoreNested(const std::string &a_key) -> std::string
{
    std::string result;
    TranslateIgnoreNested(a_key, result);
    return result;
}
} // namespace Translation
} // namespace LIBC_NAMESPACE_DECL
