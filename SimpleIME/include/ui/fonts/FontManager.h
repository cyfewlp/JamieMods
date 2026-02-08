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
public:
    using Index = int32_t;

private:
    Index       index = -1; // index in font set
    std::string name;

public:
    FontInfo(const Index index, const std::string &name) : index(index), name(name) {}

    bool IsInvalid() const
    {
        return index == -1;
    }

    auto GetName() const -> const std::string &
    {
        return name;
    }

    auto GetIndex() const -> Index
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
} // namespace Ime
