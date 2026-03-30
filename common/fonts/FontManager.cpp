//
// Created by jamie on 2026/1/11.
//
#include "FontManager.h"

#include "WCharUtils.h"

#include <dwrite_3.h>
#include <windows.h>
#include <wrl/client.h>

#pragma comment(lib, "dwrite.lib")

using Microsoft::WRL::ComPtr;

namespace Fonts
{

namespace
{
auto GetDWriteFactory3() -> ComPtr<IDWriteFactory3>
{
    ComPtr<IDWriteFactory3> factory;
    const auto              hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), &factory);
    if (FAILED(hr)) return nullptr;
    return factory;
}

auto GetFontRef(const FontInfo &fontInfo) -> ComPtr<IDWriteFontFaceReference>
{
    if (fontInfo.IsInvalid()) return nullptr;

    const ComPtr<IDWriteFactory3> factory = GetDWriteFactory3();
    if (factory == nullptr) return nullptr;

    ComPtr<IDWriteFontSet> fontSet;
    if (FAILED(factory->GetSystemFontSet(&fontSet))) return nullptr;

    ComPtr<IDWriteFontFaceReference> fontRef;
    if (FAILED(fontSet->GetFontFaceReference(static_cast<uint32_t>(fontInfo.GetIndex()), &fontRef)))
    {
        return nullptr;
    }
    return fontRef;
}

auto GetFontFilePath(IDWriteFontFile *dWriteFontFile) -> std::wstring
{
    ComPtr<IDWriteFontFileLoader> loader;
    if (FAILED(dWriteFontFile->GetLoader(&loader))) return {};

    ComPtr<IDWriteLocalFontFileLoader> localLoader;
    if (FAILED(loader.As(&localLoader))) return {};

    const void *key     = nullptr;
    UINT32      keySize = 0;
    if (SUCCEEDED(dWriteFontFile->GetReferenceKey(&key, &keySize)) && keySize > 0)
    {
        UINT32 pathLen = 0;
        if (SUCCEEDED(localLoader->GetFilePathLengthFromKey(key, keySize, &pathLen)) && pathLen > 0)
        {
            std::wstring filePath;
            filePath.resize(pathLen + 1);
            if (SUCCEEDED(localLoader->GetFilePathFromKey(key, keySize, filePath.data(), pathLen + 1)))
            {
                filePath.resize(pathLen); // remove the extra null terminator
                return filePath;
            }
        }
    }
    return {};
}

auto GetFontFilePath(IDWriteFontFaceReference *fontRef) -> std::wstring
{
    ComPtr<IDWriteFontFile> fontFile;
    if (SUCCEEDED(fontRef->GetFontFile(&fontFile)))
    {
        return GetFontFilePath(fontFile.Get());
    }
    return {};
}

auto GetFontFilePath(IDWriteFont *dWriteFont) -> std::wstring
{
    ComPtr<IDWriteFontFace> pFontFace = nullptr;
    if (SUCCEEDED(dWriteFont->CreateFontFace(&pFontFace)))
    {
        UINT32 availableFileCount = 0;
        if (SUCCEEDED(pFontFace->GetFiles(&availableFileCount, nullptr)) && availableFileCount > 0)
        {
            UINT32                  requestFileCount = 1; // we only need the first file path, even if there are multiple files for this font face
            ComPtr<IDWriteFontFile> fontFile;
            if (SUCCEEDED(pFontFace->GetFiles(&requestFileCount, &fontFile)))
            {
                return GetFontFilePath(fontFile.Get());
            }
        }
    }
    return {};
}

auto GetLocalizedString(IDWriteLocalizedStrings *pStrings) -> std::wstring
{
    HRESULT hr     = S_OK;
    UINT32  index  = 0;
    BOOL    exists = FALSE;

    std::array<wchar_t, LOCALE_NAME_MAX_LENGTH> localeName{};

    // Get the default locale for this user.

    // If the default locale is returned, find that locale name, otherwise use "en-us".
    if (FALSE == GetUserDefaultLocaleName(localeName.data(), localeName.size()))
    {
        hr = pStrings->FindLocaleName(localeName.data(), &index, &exists);
    }
    if (SUCCEEDED(hr) && FALSE == exists) // if the above find did not find a match, retry with US English
    {
        hr = pStrings->FindLocaleName(L"en-us", &index, &exists);
    }

    // If the specified locale doesn't exist, select the first on the list.
    if (FALSE == exists) index = 0;

    UINT32 length = 0;
    if (SUCCEEDED(hr))
    {
        hr = pStrings->GetStringLength(index, &length);
    }
    if (SUCCEEDED(hr) && length > 0)
    {
        std::wstring wstring(length, L'\0');
        if (SUCCEEDED(pStrings->GetString(index, wstring.data(), length + 1)))
        {
            return wstring;
        }
    }
    return {};
}

auto FindInstalledFonts() -> std::vector<FontInfo>
{
    std::vector<FontInfo>   fontList;
    ComPtr<IDWriteFactory3> factory;
    HRESULT                 hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), &factory);
    if (FAILED(hr)) return fontList;

    ComPtr<IDWriteFontSet> fontSet;
    hr = factory->GetSystemFontSet(&fontSet);
    if (FAILED(hr)) return fontList;

    const UINT32 fontCount = fontSet->GetFontCount();
    for (UINT32 idx = 0; idx < fontCount; idx++)
    {
        BOOL                            exists = FALSE;
        ComPtr<IDWriteLocalizedStrings> localizedStrings;

        if (SUCCEEDED(fontSet->GetPropertyValues(idx, DWRITE_FONT_PROPERTY_ID_FULL_NAME, &exists, &localizedStrings)))
        {
            const auto fontFullName = GetLocalizedString(localizedStrings.Get());
            if (fontFullName.empty())
            {
                continue;
            }
            fontList.emplace_back(static_cast<int32_t>(idx), WCharUtils::ToString(fontFullName));
        }
    }
    return fontList;
}

} // namespace

FontManager::FontManager()
{
    m_fontList = FindInstalledFonts();
}

auto GetFontFilePath(const FontInfo &fontInfo) -> std::wstring
{
    if (auto fontRef = GetFontRef(fontInfo); fontRef != nullptr)
    {
        return GetFontFilePath(fontRef.Get());
    }
    return {};
}

auto GetDefaultFontFilePath() -> std::wstring
{
    const ComPtr<IDWriteFactory3> dWriteFactory = GetDWriteFactory3();
    if (dWriteFactory == nullptr) return {};

    NONCLIENTMETRICS ncm = {};
    ncm.cbSize           = sizeof(ncm);
    if (FALSE == SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
    {
        return {};
    }

    ComPtr<IDWriteGdiInterop> gdiInterop = nullptr;
    if (SUCCEEDED(dWriteFactory->GetGdiInterop(&gdiInterop)))
    {
        ComPtr<IDWriteFont> font;
        if (SUCCEEDED(gdiInterop->CreateFontFromLOGFONT(&ncm.lfMessageFont, &font)))
        {
            return GetFontFilePath(font.Get());
        }
    }
    return {};
}

auto GetFirstFontFilePathInFamily(std::wstring_view familyName) -> std::wstring
{
    const ComPtr<IDWriteFactory3> dWriteFactory = GetDWriteFactory3();
    if (dWriteFactory == nullptr) return {};

    ComPtr<IDWriteFontCollection> fontCollection = nullptr;
    if (SUCCEEDED(dWriteFactory->GetSystemFontCollection(&fontCollection, FALSE)))
    {
        UINT32 index  = 0;
        BOOL   exists = FALSE;
        if (SUCCEEDED(fontCollection->FindFamilyName(std::wstring(familyName).c_str(), &index, &exists)) && exists != FALSE)
        {
            ComPtr<IDWriteFontFamily> fontFamily;
            if (SUCCEEDED(fontCollection->GetFontFamily(index, &fontFamily)))
            {
                ComPtr<IDWriteFont> firstFont;
                if (SUCCEEDED(fontFamily->GetFont(0, &firstFont)))
                {
                    return GetFontFilePath(firstFont.Get());
                }
            }
        }
    }
    return {};
}

} // namespace Fonts
