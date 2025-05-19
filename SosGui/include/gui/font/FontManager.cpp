//
// Created by jamie on 2025/5/16.
//

#include "FontManager.h"

#include "../UiSettings.h"
#include "FontInfo.h"
#include "Translation.h"
#include "common/WCharUtils.h"
#include "common/imgui/ImGuiScope.h"
#include "common/log.h"
#include "imgui.h"
#include "misc/freetype/imgui_freetype.h"

#include <dwrite_3.h>
#include <windows.h>

#pragma comment(lib, "dwrite.lib")

namespace LIBC_NAMESPACE_DECL
{

void FontManager::GetDefaultFont(const ComPtr<IDWriteFactory> &pDWriteFactory, IDWriteFont **ppFont)
{
    NONCLIENTMETRICS ncm = {};
    ncm.cbSize           = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

    ComPtr<IDWriteGdiInterop> pGdiInterop = nullptr;
    if (SUCCEEDED(pDWriteFactory->GetGdiInterop(&pGdiInterop)))
    {
        if (FAILED(pGdiInterop->CreateFontFromLOGFONT(&ncm.lfMessageFont, ppFont)))
        {
            *ppFont = nullptr;
        }
    }
}

void FontManager::Initialize()
{
    static bool initialized = false;
    if (initialized)
    {
        log_warn("FontManager already initialized, ignore call.");
        return;
    }
    try
    {
        ComPtr<IDWriteFactory> pDWriteFactory = nullptr;
        if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &pDWriteFactory)))
        {
            log_warn("Can't create DWrite factory!.");
            return;
        }
        DWriteFindAllInstalledFonts(pDWriteFactory.Get());

        auto &io = ImGui::GetIO();

        const auto *settings = Settings::UiSettings::GetInstance();

        ComPtr<IDWriteFont> pDefaultFont;
        GetDefaultFont(pDWriteFactory, &pDefaultFont);
        if (!pDefaultFont)
        {
            log_error("Can't get system default font, back to ImGui default font.");
            return;
        }

        SetupFontInfo(m_defaultFontInfo, pDefaultFont.Get());
        if (m_defaultFontInfo.IsInValid())
        {
            log_warn("Can't find system default font information, back to ImGui default font.");
            io.Fonts->AddFontDefault();
            return;
        }

        ImFontConfig config;
        config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
        config.MergeMode          = true;
        const auto  iconFontPath  = util::GetInterfaceFile(Settings::UiSettings::ICON_FONT);
        const float pixelFontSize = Settings::UiSettings::GetInstance()->FONT_PX_TEXT;

        CreateFontInfoFrom(pDWriteFactory.Get(), settings->fontInfo, m_fontInfo);
        if (m_fontInfo.IsValid())
        {
            io.Fonts->AddFontFromFileTTF(iconFontPath.c_str(), pixelFontSize);

            SetupFontConfig(config, m_fontInfo);
            io.Fonts->AddFontFromFileTTF(m_fontInfo.filePath.c_str(), pixelFontSize, &config);

            ImFontConfig config1;
            config1.MergeMode = true;
            SetupFontConfig(config1, pDefaultFont.Get());
            m_font = io.Fonts->AddFontFromFileTTF(m_defaultFontInfo.filePath.c_str(), pixelFontSize, &config1);
        }
        else // setup ImGui font from default system font;
        {
            m_fontInfo = m_defaultFontInfo;

            io.Fonts->AddFontFromFileTTF(iconFontPath.c_str(), pixelFontSize);
            m_font = io.Fonts->AddFontFromFileTTF(m_defaultFontInfo.filePath.c_str(), pixelFontSize, &config);
        }

        if (m_font)
        {
            io.FontDefault = m_font;
        }
        else
        {
            m_fontInfo.MarkInvalid();
            m_font = nullptr;
            io.Fonts->Clear();
            io.Fonts->AddFontDefault();
        }
        initialized = true;
    }
    catch (const Error &e)
    {
        log_error("Failed initializing FontManger: {}", e.what());
    }
}

constexpr auto weightToString = [](const DWRITE_FONT_WEIGHT a_weight) {
    switch (a_weight)
    {
        case DWRITE_FONT_WEIGHT_THIN:
            return "Thin";
        case DWRITE_FONT_WEIGHT_EXTRA_LIGHT: // DWRITE_FONT_WEIGHT_ULTRA_LIGHT
            return "ExtraLight";
        case DWRITE_FONT_WEIGHT_LIGHT:
            return "Light";
        case DWRITE_FONT_WEIGHT_SEMI_LIGHT:
            return "SemiLight";
        case DWRITE_FONT_WEIGHT_NORMAL: // DWRITE_FONT_WEIGHT_REGULAR
            return "Normal";
        case DWRITE_FONT_WEIGHT_MEDIUM:
            return "Medium";
        case DWRITE_FONT_WEIGHT_DEMI_BOLD: // DWRITE_FONT_WEIGHT_SEMI_BOLD
            return "DemiBold";
        case DWRITE_FONT_WEIGHT_BOLD:
            return "Bold";
        case DWRITE_FONT_WEIGHT_EXTRA_BOLD: // DWRITE_FONT_WEIGHT_ULTRA_BOLD
            return "ExtraBold";
        case DWRITE_FONT_WEIGHT_BLACK: // DWRITE_FONT_WEIGHT_HEAVY
            return "Black";
        case DWRITE_FONT_WEIGHT_EXTRA_BLACK: // DWRITE_FONT_WEIGHT_ULTRA_BLACK
            return "ExtraBlack";
    };
    return "Unknown";
};
constexpr auto styleToString = [](const DWRITE_FONT_STYLE a_style) {
    switch (a_style)
    {
        case DWRITE_FONT_STYLE_NORMAL:
            return "Normal";
        case DWRITE_FONT_STYLE_OBLIQUE:
            return "Oblique";
        case DWRITE_FONT_STYLE_ITALIC:
            return "Italic";
    }
    return "Unknown";
};
constexpr auto stretchToString = [](const DWRITE_FONT_STRETCH aStretch) {
    switch (aStretch)
    {
        case DWRITE_FONT_STRETCH_UNDEFINED:
            return "Undefined";
        case DWRITE_FONT_STRETCH_ULTRA_CONDENSED:
            return "UltraCondensed";
        case DWRITE_FONT_STRETCH_EXTRA_CONDENSED:
            return "ExtraCondensed";
        case DWRITE_FONT_STRETCH_CONDENSED:
            return "Condensed";
        case DWRITE_FONT_STRETCH_SEMI_CONDENSED:
            return "SemiCondensed";
        case DWRITE_FONT_STRETCH_NORMAL:
            return "Normal";
        case DWRITE_FONT_STRETCH_SEMI_EXPANDED:
            return "SemiExpanded";
        case DWRITE_FONT_STRETCH_EXPANDED:
            return "Expanded";
        case DWRITE_FONT_STRETCH_EXTRA_EXPANDED:
            return "ExtraExpanded";
        case DWRITE_FONT_STRETCH_ULTRA_EXPANDED:
            return "UltraExpanded";
    }
    return "Unknown";
};

void FontManager::RebuildPreviewFont(const ComPtr<IDWriteFont> &pFont)
{
    const auto &io = ImGui::GetIO();
    if (m_previewFont)
    {
        io.Fonts->RemoveFont(m_previewFont);
        m_previewFont = nullptr;
    }
    ImFontConfig config;
    config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    config.MergeMode = true;

    const auto iconFontPath = util::GetInterfaceFile(Settings::UiSettings::ICON_FONT);

    const float pixelFontSize = Settings::UiSettings::GetInstance()->FONT_PX_TEXT;
    m_previewFont             = io.Fonts->AddFontFromFileTTF(iconFontPath.c_str(), pixelFontSize);

    const std::string utf8Path = GetFontFilePath(pFont.Get());
    if (m_previewFont && utf8Path.size() > 0)
    {
        SetupFontConfig(config, pFont.Get());
        m_previewFont = io.Fonts->AddFontFromFileTTF(utf8Path.c_str(), pixelFontSize, &config);
    }
    if (m_previewFont)
    {
        ImFontConfig config1;
        config1.MergeMode = true;
        SetupFontConfig(config1, m_defaultFontInfo);
        m_previewFont = io.Fonts->AddFontFromFileTTF(m_defaultFontInfo.filePath.c_str(), pixelFontSize, &config1);
    }
}

void FontManager::DrawPanel()
{
    auto preview = "";
    if (m_fontInfo.familyIndex >= 0 && static_cast<size_t>(m_fontInfo.familyIndex) < m_fontFamilies.size())
    {
        preview = m_fontFamilies[m_fontInfo.familyIndex].familyName.c_str();
    }

    bool apply = false;
    {
        bool rebuildFont = false;
        auto group       = ImGuiScope::Group();
        if (ImGui::BeginCombo("$SosGui_Font"_T.c_str(), preview))
        {
            for (size_t index = 0; index < m_fontFamilies.size(); index++)
            {
                const bool selected = static_cast<size_t>(m_fontInfo.familyIndex) == index;
                if (ImGui::Selectable(m_fontFamilies[index].familyName.c_str(), selected) && !selected)
                {
                    m_fontInfo.familyIndex = index;
                    m_fontInfo.fontIndex   = 0;
                    rebuildFont            = true;
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        auto labelFromFont = [](IDWriteFont *pFont) {
            ComPtr<IDWriteLocalizedStrings> pInformation;
            BOOL                            exists = FALSE;
            std::string                     fullname;
            if (SUCCEEDED(pFont->GetInformationalStrings(DWRITE_INFORMATIONAL_STRING_FULL_NAME, &pInformation, &exists)
                ) &&
                exists)
            {
                if (auto information = GetLocalizedString(pInformation.Get()); information.size() > 0)
                {
                    fullname = std::move(information);
                }
            }
            else
                return std::string("Unknown");
            return std::format(
                "{} ({} / {} / {})",
                fullname,
                weightToString(pFont->GetWeight()),
                styleToString(pFont->GetStyle()),
                stretchToString(pFont->GetStretch())
            );
        };
        if (m_fontInfo.familyIndex >= 0)
        {
            const auto         &fontFamily = m_fontFamilies[m_fontInfo.familyIndex];
            ComPtr<IDWriteFont> pPreviewFont;
            std::string         previewFont;
            if (SUCCEEDED(fontFamily.pFontFamily->GetFont(m_fontInfo.fontIndex, &pPreviewFont)))
            {
                previewFont = labelFromFont(pPreviewFont.Get());
            }
            if (ImGui::BeginCombo("Typography", previewFont.c_str(), ImGuiComboFlags_HeightLarge))
            {
                for (size_t index = 0; index < fontFamily.pFontFamily->GetFontCount(); index++)
                {
                    ImGuiScope::PushId pushId(index);
                    std::string        label;
                    if (ComPtr<IDWriteFont> pFont; SUCCEEDED(fontFamily.pFontFamily->GetFont(index, &pFont)))
                    {
                        label = labelFromFont(pFont.Get());
                    }
                    const bool selected = static_cast<size_t>(m_fontInfo.fontIndex) == index;
                    if (ImGui::Selectable(label.c_str(), selected) && !selected)
                    {
                        m_fontInfo.fontIndex = index;
                        rebuildFont          = true;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (rebuildFont)
            {
                if (ComPtr<IDWriteFont> pFont; SUCCEEDED(fontFamily.pFontFamily->GetFont(m_fontInfo.fontIndex, &pFont)))
                {
                    RebuildPreviewFont(pFont.Get());
                }
            }
        }
        apply = ImGui::Button("$SosGui_Apply"_T.c_str());
    }

    // preview
    {
        ImGui::SameLine();
        auto group = ImGuiScope::Group();
        if (m_previewFont) ImGui::PushFont(m_previewFont);
        ImGui::Text("%s", R"(abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKLMNOPQRSTUVWXYZ
0123456789 (){}[]
+ - * / = .,;:!? #&$%@|^)");
        ImGui::Text("The quick brown fox jumps over the lazy dog");

        ImGui::Text(
            "Emoji: "
            "🥰💀✌︎🌴🐢🐐🍄⚽🍻👑📸😬👀🚨🏡🐦‍🔥🍋‍🟩🍄‍🟫🙂‍"
        );
        ImGui::Text("Chinese: 快速的棕色狐狸跳过了懒惰的狗");
        ImGui::Text("Japanese: 速い茶色のキツネが怠惰な犬を飛び越えます");
        ImGui::Text("Korean: 빠른 갈색 여우가 게으른 개를 뛰어넘습니다");
        if (m_previewFont) ImGui::PopFont();
    }

    if (apply && m_previewFont)
    {
        auto &io = ImGui::GetIO();

        if (m_font) io.Fonts->RemoveFont(m_font);
        io.FontDefault = m_previewFont;
        m_font         = m_previewFont;
        m_previewFont  = nullptr;
    }

    // TODO ImGui::Text("$SosGui_Font_Size"_T.c_str());
    // ImGui::SameLine();
}

void FontManager::SyncSettings(Settings::UiSettings *uiSetting) const noexcept
{
    uiSetting->fontInfo.filePath    = m_fontInfo.filePath;
    uiSetting->fontInfo.familyName  = m_fontInfo.familyName;
    uiSetting->fontInfo.familyIndex = m_fontInfo.familyIndex;
    uiSetting->fontInfo.fontIndex   = m_fontInfo.fontIndex;
    uiSetting->fontInfo.faceIndex   = m_fontInfo.faceIndex;
    uiSetting->fontInfo.bold        = m_fontInfo.bold;
    uiSetting->fontInfo.oblique     = m_fontInfo.oblique;
}

void FontManager::DWriteFindAllInstalledFonts(IDWriteFactory *pDWriteFactory) noexcept
{
    ComPtr<IDWriteFontCollection> pFontCollection = nullptr;
    auto                          hr              = pDWriteFactory->GetSystemFontCollection(&pFontCollection);
    if (FAILED(hr))
    {
        return;
    }

    UINT32 familyCount = 0;

    // Get the number of font families in the collection.
    if (SUCCEEDED(hr))
    {
        familyCount = pFontCollection->GetFontFamilyCount();
    }

    m_fontFamilies.reserve(familyCount);
    for (UINT32 i = 0; i < familyCount; ++i)
    {
        ComPtr<IDWriteFontFamily> pFontFamily = nullptr;
        if (hr = pFontCollection->GetFontFamily(i, &pFontFamily); SUCCEEDED(hr))
        {
            ComPtr<IDWriteLocalizedStrings> pFamilyNames = nullptr;

            // Get a list of localized strings for the family name.
            if (hr = pFontFamily->GetFamilyNames(&pFamilyNames); FAILED(hr))
            {
                continue;
            }

            auto name = GetLocalizedString(pFamilyNames.Get());
            if (name.empty())
            {
                continue;
            }
            m_fontFamilies.emplace_back(name, pFontFamily);
        }
    }
}

auto FontManager::GetLocalizedString(IDWriteLocalizedStrings *pStrings) -> std::string
{
    HRESULT hr     = S_OK;
    UINT32  index  = 0;
    BOOL    exists = false;

    wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

    // Get the default locale for this user.

    // If the default locale is returned, find that locale name, otherwise use "en-us".
    if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH))
    {
        hr = pStrings->FindLocaleName(localeName, &index, &exists);
    }
    if (SUCCEEDED(hr) && !exists) // if the above find did not find a match, retry with US English
    {
        hr = pStrings->FindLocaleName(L"en-us", &index, &exists);
    }

    // If the specified locale doesn't exist, select the first on the list.
    if (!exists) index = 0;

    UINT32 length = 0;

    // Get the string length.
    if (SUCCEEDED(hr))
    {
        hr = pStrings->GetStringLength(index, &length);
    }
    std::wstring stdString;
    stdString.resize(length + 1);
    // Get the family name.
    if (SUCCEEDED(hr))
    {
        hr = pStrings->GetString(index, stdString.data(), length + 1);
    }
    if (SUCCEEDED(hr))
    {
        return WCharUtils::ToString(stdString.c_str(), static_cast<int>(length));
    }
    return std::string();
}

auto FontManager::GetFontFilePath(IDWriteFont *pFont) -> std::string
{
    std::string             result;
    ComPtr<IDWriteFontFace> pFontFace = nullptr;
    if (FAILED(pFont->CreateFontFace(&pFontFace)))
    {
        return result;
    }

    UINT32 numFiles = 0;
    if (FAILED(pFontFace->GetFiles(&numFiles, nullptr)))
    {
        return result;
    }

    std::vector<IDWriteFontFile *> fontFiles(numFiles);
    if (FAILED(pFontFace->GetFiles(&numFiles, fontFiles.data())))
    {
        return result;
    }

    IDWriteFontFile *fontFile         = fontFiles[0];
    const void      *referenceKey     = nullptr;
    UINT32           referenceKeySize = 0;
    if (FAILED(fontFile->GetReferenceKey(&referenceKey, &referenceKeySize)))
    {
        return result;
    }

    ComPtr<IDWriteFontFileLoader> baseLoader;
    if (FAILED(fontFile->GetLoader(&baseLoader)))
    {
        return result;
    }

    ComPtr<IDWriteLocalFontFileLoader> localLoader;
    if (FAILED(baseLoader.As(&localLoader)))
    {
        return result;
    }

    UINT32 pathLength = 0;
    if (SUCCEEDED(localLoader->GetFilePathLengthFromKey(referenceKey, referenceKeySize, &pathLength)))
    {
        std::wstring pathBuffer(pathLength + 1, '\0');
        if (SUCCEEDED(localLoader->GetFilePathFromKey(referenceKey, referenceKeySize, pathBuffer.data(), pathLength + 1)
            ))
        {
            result = SKSE::stl::utf16_to_utf8(pathBuffer).value_or("");
        }
    }

    std::ranges::for_each(fontFiles, [&](IDWriteFontFile *pFontFile) {
        pFontFile->Release();
    });
    return result;
}

auto FontManager::GetFontFamily(
    IDWriteFactory *pDWriteFactory, const std::string &familyName, IDWriteFontFamily **ppFontFamily
) -> bool
{
    *ppFontFamily         = nullptr;
    const auto &utf16Name = SKSE::stl::utf8_to_utf16(familyName).value_or(L"");
    if (utf16Name.empty())
    {
        return false;
    }
    if (ComPtr<IDWriteFontCollection> pFontCollection = nullptr;
        SUCCEEDED(pDWriteFactory->GetSystemFontCollection(&pFontCollection)))
    {
        uint32_t index  = 0;
        BOOL     exists = FALSE;
        if (SUCCEEDED(pFontCollection->FindFamilyName(utf16Name.c_str(), &index, &exists)) && exists)
        {
            return SUCCEEDED(pFontCollection->GetFontFamily(index, ppFontFamily));
        }
    }
    return false;
}

void FontManager::CreateFontInfoFrom(IDWriteFactory *pFactory, const FontInfo &source, FontInfo &dest)
{
    dest.MarkInvalid();
    if (source.IsInValid())
    {
        return;
    }
    ComPtr<IDWriteFontFamily> pFontFamily;
    if (GetFontFamily(pFactory, source.familyName, &pFontFamily) && pFontFamily)
    {
        // check font index is valid;
        if (static_cast<uint32_t>(source.fontIndex) >= pFontFamily->GetFontCount())
        {
            return;
        }

        dest = source;
    }
}

void FontManager::SetupFontConfig(ImFontConfig &config, IDWriteFont *pFont)
{
    if (ComPtr<IDWriteFontFace> pFontFace = nullptr; SUCCEEDED(pFont->CreateFontFace(&pFontFace)))
    {
        config.FontNo = pFontFace->GetIndex();
    }

    if (pFont->GetStyle() == DWRITE_FONT_STYLE_OBLIQUE || pFont->GetSimulations() == DWRITE_FONT_SIMULATIONS_OBLIQUE)
    {
        config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Oblique;
    }
    if (pFont->GetWeight() >= DWRITE_FONT_WEIGHT_BOLD || pFont->GetSimulations() == DWRITE_FONT_SIMULATIONS_BOLD)
    {
        config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bold;
    }
}

void FontManager::SetupFontConfig(ImFontConfig &config, const FontInfo &fontInfo)
{
    config.FontNo = fontInfo.faceIndex;
    if (fontInfo.bold)
    {
        config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bold;
    }
    if (fontInfo.oblique)
    {
        config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Oblique;
    }
}

void FontManager::SetupFontInfo(FontInfo &fontInfo, IDWriteFont *pFont) const
{
    fontInfo.filePath = GetFontFilePath(pFont);
    if (ComPtr<IDWriteFontFamily> pDefaultFontFamily; SUCCEEDED(pFont->GetFontFamily(&pDefaultFontFamily)))
    {
        if (ComPtr<IDWriteLocalizedStrings> pFamilyNames; SUCCEEDED(pDefaultFontFamily->GetFamilyNames(&pFamilyNames)))
        {
            fontInfo.familyName = GetLocalizedString(pFamilyNames.Get());
            for (size_t index = 0; index < m_fontFamilies.size(); index++)
            {
                if (fontInfo.familyName == m_fontFamilies[index].familyName)
                {
                    fontInfo.familyIndex = index;
                    break;
                }
            }
            if (ComPtr<IDWriteFontFace> pFontFace; SUCCEEDED(pFont->CreateFontFace(&pFontFace)))
            {
                fontInfo.bold = pFont->GetWeight() >= DWRITE_FONT_WEIGHT_BOLD ||
                                pFont->GetSimulations() == DWRITE_FONT_SIMULATIONS_BOLD;
                fontInfo.oblique = pFont->GetStyle() == DWRITE_FONT_STYLE_OBLIQUE ||
                                   pFont->GetSimulations() == DWRITE_FONT_SIMULATIONS_OBLIQUE;
            }
        }
        const auto fontCount = pDefaultFontFamily->GetFontCount();
        for (uint32_t fontIndex = 0; fontIndex < fontCount; fontIndex++)
        {
            ComPtr<IDWriteFont3> pFont3;
            if (ComPtr<IDWriteFont> pFont0;
                SUCCEEDED(pDefaultFontFamily->GetFont(fontIndex, &pFont0)) && SUCCEEDED(pFont0.As(&pFont3)))
            {
                if (pFont3->Equals(pFont))
                {
                    fontInfo.fontIndex = fontIndex;
                    if (ComPtr<IDWriteFontFace> pFontFace; SUCCEEDED(pFont->CreateFontFace(&pFontFace)))
                    {
                        fontInfo.faceIndex = pFontFace->GetIndex();
                    }
                    return;
                }
            }
        }
    }
    fontInfo.MarkInvalid();
}

Settings::UiSettings::UiSettings()
{
    const HDC hdc  = GetDC(nullptr);
    const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(nullptr, hdc);

    FONT_PX_TEXT_SMALL = MulDiv(FONT_PT_TEXT_SMALL, dpiY, 72);
    FONT_PX_TEXT       = MulDiv(FONT_PT_TEXT, dpiY, 72);
    FONT_PX_TITLE_1    = MulDiv(FONT_PT_TITLE_1, dpiY, 72);
    FONT_PX_TITLE_2    = MulDiv(FONT_PT_TITLE_2, dpiY, 72);
    FONT_PX_TITLE_3    = MulDiv(FONT_PT_TITLE_3, dpiY, 72);
    FONT_PX_TITLE_4    = MulDiv(FONT_PT_TITLE_4, dpiY, 72);
}
}
