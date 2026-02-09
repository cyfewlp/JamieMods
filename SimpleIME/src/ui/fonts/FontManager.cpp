//
// Created by jamie on 2026/1/11.
//
#include "ui/fonts/FontManager.h"

#include "WCharUtils.h"
#include "log.h"
#include "toml/toml.hpp"

#include <dwrite_3.h>
#include <windows.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace Ime
{

using Microsoft::WRL::ComPtr;

namespace
{
auto GetFontRef(const FontInfo &fontInfo) -> ComPtr<IDWriteFontFaceReference>
{
    if (fontInfo.IsInvalid()) return nullptr;

    ComPtr<IDWriteFactory3> factory;

    HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), &factory);
    if (FAILED(hr)) return nullptr;

    ComPtr<IDWriteFontSet> fontSet;
    hr = factory->GetSystemFontSet(&fontSet);
    if (FAILED(hr)) return nullptr;

    ComPtr<IDWriteFontFaceReference> fontRef;
    hr = fontSet->GetFontFaceReference(static_cast<uint32_t>(fontInfo.GetIndex()), &fontRef);
    if (FAILED(hr)) return nullptr;
    return fontRef;
}

auto GetPathFromReference(IDWriteFontFaceReference *fontRef) -> std::wstring
{
    std::wstring            filePath;
    ComPtr<IDWriteFontFile> dwFontFile;
    HRESULT                 hr = fontRef->GetFontFile(&dwFontFile);
    if (FAILED(hr)) return filePath;

    ComPtr<IDWriteFontFileLoader> loader;
    hr = dwFontFile->GetLoader(&loader);
    if (FAILED(hr)) return filePath;

    ComPtr<IDWriteLocalFontFileLoader> localLoader;
    if (FAILED(loader.As(&localLoader))) return filePath;

    const void *key     = nullptr;
    UINT32      keySize = 0;
    if (SUCCEEDED(dwFontFile->GetReferenceKey(&key, &keySize)) && keySize > 0)
    {
        UINT32 pathLen = 0;
        hr             = localLoader->GetFilePathLengthFromKey(key, keySize, &pathLen);
        if (SUCCEEDED(hr) && pathLen > 0)
        {
            filePath.resize(pathLen + 1);
            hr = localLoader->GetFilePathFromKey(key, keySize, filePath.data(), pathLen + 1);
            if (SUCCEEDED(hr))
            {
                filePath.resize(pathLen); // remove the extra null terminator
                return filePath;
            }
        }
    }
    return filePath;
}
} // namespace

void FontManager::FindInstalledFonts()
{
    if (!m_fontList.empty())
    {
        logger::warn("Already fill all installed fonts info.");
        return;
    }

    ComPtr<IDWriteFactory3> factory;

    HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), &factory);
    if (FAILED(hr)) return;

    ComPtr<IDWriteFontSet> fontSet;
    hr = factory->GetSystemFontSet(&fontSet);
    if (FAILED(hr)) return;

    const UINT32 fontCount = fontSet->GetFontCount();
    for (UINT32 idx = 0; idx < fontCount; idx++)
    {
        BOOL                            exists = FALSE;
        ComPtr<IDWriteLocalizedStrings> localizedStrings;

        if (SUCCEEDED(fontSet->GetPropertyValues(idx, DWRITE_FONT_PROPERTY_ID_FULL_NAME, &exists, &localizedStrings)))
        {
            std::string fontFullName;
            GetLocalizedString(localizedStrings.Get(), fontFullName);
            if (fontFullName.empty())
            {
                continue;
            }
            m_fontList.emplace_back(FontInfo(idx, fontFullName));
        }
    }
}

auto FontManager::GetFontFilePath(const FontInfo &fontInfo) -> std::string
{
    std::string result;

    auto fontRef = GetFontRef(fontInfo);
    if (!fontRef) return result;

    const auto &filePath = GetPathFromReference(fontRef.Get());
    return WCharUtils::ToString(filePath);
}

void FontManager::GetLocalizedString(IDWriteLocalizedStrings *pStrings, std::string &result)
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
    std::wstring wstring(length, L'\0');
    // Get the family name.
    if (SUCCEEDED(hr))
    {
        hr = pStrings->GetString(index, wstring.data(), length + 1);
    }
    if (SUCCEEDED(hr))
    {
        result = WCharUtils::ToString(wstring);
    }
}
} // namespace Ime
