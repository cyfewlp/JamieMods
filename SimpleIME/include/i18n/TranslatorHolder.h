//
// Created by jamie on 2026/1/29.
//

#pragma once

#include "common/i18n/Translator.h"

#include <atomic>
#include <memory>

namespace Ime
{

class TranslatorHolder
{

public:
    class UpdateHandle
    {
        friend class TranslatorHolder;
        i18n::Translator &m_ref;

        explicit UpdateHandle(i18n::Translator &translator) : m_ref(translator) {}

    public:
        void Update(i18n::Translator &&newTranslator) const
        {
            m_ref = std::move(newTranslator);
        }
    };

private:
    static inline auto g_translator  = i18n::Translator();
    static inline auto g_initialized = std::atomic_bool(false);

public:
    // Only the first caller will obtain UpdateHandle.
    static auto RequestUpdateHandle() -> std::optional<UpdateHandle>
    {
        bool expected = false;
        if (g_initialized.compare_exchange_strong(expected, true))
        {
            return UpdateHandle(g_translator);
        }
        return std::nullopt;
    }

    static auto GetTranslator() -> const i18n::Translator &
    {
        return g_translator;
    }
};

constexpr auto Translate(const std::string_view key) -> const char *
{
    return TranslatorHolder::GetTranslator().Translate(i18n::HashKey(key), key).data();
}
}
