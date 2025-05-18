//
// Created by jamie on 2025/5/18.
//

#ifndef FONTINFO_H
#define FONTINFO_H

#include "common/config.h"

#include <string_view>

namespace LIBC_NAMESPACE_DECL
{
struct FontInfo
{
    std::string filePath;
    std::string familyName;
    int32_t     familyIndex = -1; // the font family index in SystemFontCollection;
    int32_t     fontIndex   = -1; // font index in family
    uint32_t    faceIndex   = 0;
    bool        bold        = false;
    bool        oblique     = false;

    FontInfo() = default;

    FontInfo(
        const std::string_view &filePath, const std::string_view &familyName, const uint32_t fontIndex,
        const bool bold = false, const bool oblique = false
    )
        : filePath(filePath), familyName(familyName), fontIndex(fontIndex), bold(bold), oblique(oblique)
    {
    }

    FontInfo(const FontInfo &other)
        : filePath(other.filePath), familyName(other.familyName), familyIndex(other.familyIndex),
          fontIndex(other.fontIndex), faceIndex(other.faceIndex), bold(other.bold), oblique(other.oblique)
    {
    }

    FontInfo(FontInfo &&other) noexcept = delete;

    FontInfo &operator=(const FontInfo &other)
    {
        if (this == &other) return *this;
        filePath    = other.filePath;
        familyName  = other.familyName;
        familyIndex = other.familyIndex;
        fontIndex   = other.fontIndex;
        faceIndex   = other.faceIndex;
        bold        = other.bold;
        oblique     = other.oblique;
        return *this;
    }

    FontInfo &operator=(FontInfo &&other) noexcept = delete;

    bool IsValid() const
    {
        return fontIndex >= 0 && filePath.size() > 0 && familyName.size() > 0;
    }

    bool IsInValid() const
    {
        return !IsValid();
    }

    void MarkInvalid()
    {
        filePath.clear();
        familyName.clear();
        familyIndex = -1;
        fontIndex   = -1;
    }
};
}

#endif // FONTINFO_H
