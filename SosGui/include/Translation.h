//
// Created by jamie on 2025/4/7.
//

#ifndef TRANSLATION_H
#define TRANSLATION_H

#pragma once

namespace LIBC_NAMESPACE_DECL
{
    namespace Translation
    {
        void Translate(const char *key, std::string &result);

        auto Translate(const char *key) -> std::string;

        template <typename... Args>
        auto Translate(const char *key, Args &&...args) -> std::string
        {
            std::string templateStr;
            Translate(key, templateStr);
            return std::vformat(templateStr, std::make_format_args(std::forward<Args>(args)...));
        }

        auto TranslateIgnoreNested(const std::string &a_key, std::string &a_result) -> bool;

        auto TranslateIgnoreNested(const std::string &a_key) -> std::string;
    }
}

#endif // TRANSLATION_H
