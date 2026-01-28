//
// Created by jamie on 2026/1/29.
//

#pragma once

#include "common/config.h"
#include "common/i18n/Translator.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

class ImeUI;

class TranslatorHolder
{
    static inline auto g_translator = i18n::Translator();

    friend class ImeUI;

public:
    static auto GetTranslator() -> const i18n::Translator &
    {
        return g_translator;
    }
};

constexpr auto Translate(const std::string_view key)
{
    return TranslatorHolder::GetTranslator().Translate(i18n::HashKey(key), key);
}
}
}
