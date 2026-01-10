//
// Created by jamie on 2026/1/11.
//
#pragma once

#include "common/config.h"

#include <string>
#include <vector>

struct IDWriteLocalizedStrings;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
struct FontInfo
{
    int32_t     index = -1; // index in font set
    std::string name;

    FontInfo(const int32_t index, const std::string &name) : index(index), name(name) {}

    bool IsInvalid() const
    {
        return index == -1;
    }
};

class FontManager
{
    std::vector<FontInfo> m_fontList;

public:
    auto GetFontInfoList() const -> const std::vector<FontInfo> &
    {
        return m_fontList;
    }

    void FindInstalledFonts();

    auto GetFontFilePath(const FontInfo &fontInfo) -> std::string;

private:
    static void GetLocalizedString(IDWriteLocalizedStrings *pStrings, std::string &result);
};
}
}
