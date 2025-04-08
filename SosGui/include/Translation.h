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

        auto TranslateIgnoreNested(const std::string &a_key, std::string &a_result) -> bool;

        auto TranslateIgnoreNested(const std::string &a_key) -> std::string;
    }
}

#endif // TRANSLATION_H
