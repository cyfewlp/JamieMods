//
// Created by jamie on 2026/1/11.
//
#pragma once

#include <string>
#include <vector>

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

    [[nodiscard]] auto IsInvalid() const -> bool { return index == -1; }

    [[nodiscard]] auto GetName() const -> const std::string & { return name; }

    [[nodiscard]] auto GetIndex() const -> Index { return index; }
};

// The performance effect is quite low even if every frame construct a new FontManager.
class FontManager
{
    std::vector<FontInfo> m_fontList;

public:
    explicit FontManager();

    [[nodiscard]] auto GetFontInfoList() const -> const std::vector<FontInfo> & { return m_fontList; }
};

auto GetFontFilePath(const FontInfo &fontInfo) -> std::string;

auto GetDefaultFontFilePath() -> std::wstring;
auto GetFirstFontFilePathInFamily(std::wstring_view familyName) -> std::wstring;

} // namespace Ime
