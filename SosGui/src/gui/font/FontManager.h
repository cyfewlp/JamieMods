//
// Created by jamie on 2025/5/16.
//

#include "App.h"
#include "FontInfo.h"
#include "gui/UiSettings.h"

#include <dwrite.h>
#include <wrl/client.h>

struct IDWriteFontFamily;
struct ImGuiIO;
struct ImFont;

using Microsoft::WRL::ComPtr;

namespace SosGui
{
// Use MergeMode: icon font + ascii font + fallback font
class FontManager
{
    class SystemFontFamily
    {
        std::unique_ptr<char[]>   familyName;
        ComPtr<IDWriteFontFamily> pFontFamily = nullptr;

    public:
        SystemFontFamily(const std::string &&familyName, const ComPtr<IDWriteFontFamily> &pFontFamily) : pFontFamily(pFontFamily)
        {
            this->familyName = std::make_unique<char[]>(familyName.size() + 1);
            familyName.copy(this->familyName.get(), familyName.length());
        }

        [[nodiscard]] constexpr auto FamilyName() const -> std::string_view { return familyName.get(); }

        [[nodiscard]] constexpr auto FontFamily() const -> const ComPtr<IDWriteFontFamily> & { return pFontFamily; }
    };

    ImFont  *m_previewFont = nullptr;
    ImFont  *m_font        = nullptr;
    FontInfo m_defaultFontInfo{};
    FontInfo m_fontInfo{}; // active FontInfo

    std::vector<SystemFontFamily> m_fontFamilies;
    std::vector<std::string>      m_fontFullNames; // a cache for selected font family

public:
    struct Error final : std::runtime_error
    {
        explicit Error(const std::string &message) : std::runtime_error(message) {}
    };

    void Initialize() noexcept(false);
    void DrawPanel() noexcept(false);
    void SyncSettings(Settings::UiSettings *uiSetting) const noexcept;

    static auto GetInstance() -> FontManager &
    {
        static FontManager instance;
        return instance;
    }

private:
    void DrawFontFamilyCombo(bool &rebuildPreviewFont) noexcept;
    void DrawPreviewText() const noexcept;

    void        DWriteFindAllInstalledFonts(IDWriteFactory *pDWriteFactory) noexcept;
    static auto GetLocalizedString(IDWriteLocalizedStrings *pStrings) -> std::string;
    static auto GetFontFilePath(IDWriteFont *pFont) -> std::string;
    static void GetAllFontFullName(IDWriteFontFamily *pFontFamily, std::vector<std::string> &result);
    static void CreateFontInfoFrom(IDWriteFactory *pFactory, const FontInfo &source, FontInfo &dest);
    static void SetupFontConfig(ImFontConfig &config, IDWriteFont *pFont);
    static void SetupFontConfig(ImFontConfig &config, const FontInfo &fontInfo);
    void        SetupFontInfo(FontInfo &fontInfo, IDWriteFont *pFont) const;
    static void GetDefaultFont(const ComPtr<IDWriteFactory> &pDWriteFactory, IDWriteFont **ppFont);
    void        RebuildPreviewFont(const ComPtr<IDWriteFont> &pFont);
    void        RebuildPreviewFont();
    void        DrawFontsCombo(bool &rebuildFont);
};
} // namespace SosGui
