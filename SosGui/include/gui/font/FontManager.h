//
// Created by jamie on 2025/5/16.
//

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "App.h"
#include "FontInfo.h"
#include "common/config.h"

#include <dwrite.h>
#include <wrl/client.h>

struct IDWriteFontFamily;
struct ImGuiIO;
struct ImFont;

namespace LIBC_NAMESPACE_DECL
{
using Microsoft::WRL::ComPtr;

// Use MergeMode: icon font + ascii font + fallback font
class FontManager
{
    struct SystemFontFamily
    {
        std::string               familyName;
        ComPtr<IDWriteFontFamily> pFontFamily = nullptr;

        SystemFontFamily(const std::string &familyName, const ComPtr<IDWriteFontFamily> &pFontFamily)
            : familyName(familyName), pFontFamily(pFontFamily)
        {
        }
    };

    struct UiData
    {
        int fontFamilyIndex = -1;
        int fontIndex       = -1;
    } m_uiData{};

    ImFont  *m_previewFont = nullptr;
    ImFont  *m_font        = nullptr;
    FontInfo m_defaultFontInfo{};
    FontInfo m_FontInfo{}; // active FontInfo

    std::vector<SystemFontFamily> m_fontFamilies;

public:
    struct Error final : std::runtime_error
    {
        explicit Error(const std::string &message) : std::runtime_error(message) {}
    };

    void Initialize() noexcept(false);
    void DrawPanel() noexcept(false);

    static auto GetInstance() -> FontManager &
    {
        static FontManager instance;
        return instance;
    }

private:
    void        DWriteFindAllInstalledFonts(IDWriteFactory *pDWriteFactory) noexcept;
    bool        SetupUiIndexFromFontInfo(const FontInfo &fontInfo);
    static auto GetLocalizedString(IDWriteLocalizedStrings *pStrings) -> std::string;
    static auto GetFontFilePath(IDWriteFont *pFont) -> std::string;
    static auto GetFontFamily(
        IDWriteFactory *pDWriteFactory, const std::string &familyName, IDWriteFontFamily **ppFontFamily
    ) -> void;
    static void CreateFontInfoFrom(IDWriteFactory *pFactory, const FontInfo &source, FontInfo &dest);
    static void SetupFontConfig(ImFontConfig &config, IDWriteFont *pFont);
    static void SetupFontConfig(ImFontConfig &config, const FontInfo &fontInfo);
    static void SetupFontInfo(FontInfo &fontInfo, IDWriteFont *pFont);
    static void GetDefaultFont(const ComPtr<IDWriteFactory> &pDWriteFactory, IDWriteFont **ppFont);
    void        RebuildPreviewFont(const ComPtr<IDWriteFont> &pFont);
};
}

#endif // FONTMANAGER_H
