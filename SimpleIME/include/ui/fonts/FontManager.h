//
// Created by jamie on 2026/1/11.
//
#pragma once

#include <string>
#include <vector>

struct IDWriteLocalizedStrings;

namespace Ime
{
class FontInfo
{
    int32_t     index = -1; // index in font set
    std::string name;

public:
    FontInfo(const int32_t index, const std::string &name) : index(index), name(name) {}

    bool IsInvalid() const
    {
        return index == -1;
    }

    auto GetName() const -> const std::string &
    {
        return name;
    }

    auto GetIndex() const -> int32_t
    {
        return index;
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

    static auto GetFontFilePath(const FontInfo &fontInfo) -> std::string;

private:
    static void GetLocalizedString(IDWriteLocalizedStrings *pStrings, std::string &result);
};
}
