//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "common/config.h"
#include "core/Translation.h"
#include "ui/Settings.h"
#include "ui/fonts/FontManager.h"

#include <string>
#include <vector>

struct ImFont;
struct ImGuiTextFilter;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

/**
 * @brief Manages preview font data manually to prevent redundant memory copying.
 * @details This implementation sets @c FontDataOwnedByAtlas to false.
 * The allocated memory is explicitly released during the preview
 * font rebuild process.
 * @note Ensure that the Atlas remains valid as long as the font data is in use.
 */
class PreviewFont
{
    ImFont     *m_imFont = nullptr;
    std::string m_filePath;
    std::string m_fullName;
    bool        m_fontOwner  = false;
    bool        m_wantUpdate = false;

public:
    constexpr bool IsCommittable() const
    {
        return m_imFont != nullptr && !m_filePath.empty();
    }

    constexpr bool IsWantUpdate() const
    {
        return m_wantUpdate;
    }

    [[nodiscard]] auto GetImFont() const -> ImFont *
    {
        return m_imFont;
    }

    [[nodiscard]] auto GetFilePath() const -> const std::string &
    {
        return m_filePath;
    }

    [[nodiscard]] auto GetFullName() const -> const std::string &
    {
        return m_fullName;
    }

    [[nodiscard]] bool IsFontOwner() const
    {
        return m_fontOwner;
    }

    void SetProperty(const std::string &a_fullName, const std::string &a_fontFilePath)
    {
        m_fullName   = a_fullName;
        m_filePath   = a_fontFilePath;
        m_wantUpdate = true;
    }

    void SetImFont(ImFont *imFont)
    {
        m_imFont     = imFont;
        m_fontOwner  = true;
        m_wantUpdate = false;
    }

    void SetPreviewImFont(ImFont *imFont)
    {
        m_imFont    = imFont;
        m_fontOwner = false;
    }

    void Reset()
    {
        m_imFont = nullptr;
        m_fullName.clear();
        m_filePath.clear();
        m_fontOwner  = false;
        m_wantUpdate = false;
    }
};

class FontBuilder
{
public:
    void Initialize()
    {
        m_fontManager.FindInstalledFonts();
    }

    void UpdatePreviewFont(const FontInfo &fontInfo);
    void BuildPreviewFont();

    void SetBaseFont();
    void MergeFont();
    bool MergeFont(ImFont *imFont, std::string_view fullName, std::string_view fontPath);
    void Preview();
    void SetAsDefault(Settings &settings);
    void Reset();

    constexpr bool IsBuilding() const
    {
        return m_baseFont != nullptr;
    }

    constexpr auto GetFontNames() -> const std::vector<std::string> &
    {
        return m_fontNames;
    }

    constexpr auto GetPreviewFont() -> const PreviewFont &
    {
        return m_previewFont;
    }

    constexpr auto GetFontManager() -> const FontManager &
    {
        return m_fontManager;
    }

private:
    void ReleasePreviewFont();

    FontManager              m_fontManager = {};
    ImFont                  *m_baseFont    = nullptr;
    PreviewFont              m_previewFont;
    std::vector<std::string> m_fontNames;
    std::vector<std::string> m_fontPathList;
};
}
}
