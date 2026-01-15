//
// Created by jamie on 2026/1/11.
//
#include "ui/fonts/FontManager.h"

#include "common/WCharUtils.h"
#include "common/log.h"

#include <dwrite_3.h>
#include <windows.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

using Microsoft::WRL::ComPtr;

void FontManager::FindInstalledFonts()
{
    if (!m_fontList.empty())
    {
        log_warn("Already fill all installed fonts info.");
        return;
    }

    ComPtr<IDWriteFactory3> factory;

    HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), &factory);
    if (FAILED(hr))
    {
        return;
    }

    ComPtr<IDWriteFontSet> fontSet;
    hr = factory->GetSystemFontSet(&fontSet);
    if (SUCCEEDED(hr))
    {
        const UINT32 fontCount = fontSet->GetFontCount();
        for (UINT32 idx = 0; idx < fontCount; idx++)
        {
            BOOL                            exists = FALSE;
            ComPtr<IDWriteLocalizedStrings> localizedStrings;

            if (SUCCEEDED(fontSet->GetPropertyValues(idx, DWRITE_FONT_PROPERTY_ID_FULL_NAME, &exists, &localizedStrings)
                ))
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
}

auto FontManager::GetFontFilePath(const FontInfo &fontInfo) -> std::string
{
    std::string result;
    if (fontInfo.IsInvalid()) return result;

    ComPtr<IDWriteFactory3> factory;
    HRESULT                 hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), &factory);
    if (FAILED(hr)) return result;

    ComPtr<IDWriteFontSet> fontSet;
    hr = factory->GetSystemFontSet(&fontSet);
    if (FAILED(hr)) return result;

    ComPtr<IDWriteFontFaceReference> fontRef;
    hr = fontSet->GetFontFaceReference(fontInfo.index, &fontRef);
    if (FAILED(hr)) return result;

    ComPtr<IDWriteFontFile> fontFile;
    hr = fontRef->GetFontFile(&fontFile);
    if (FAILED(hr)) return result;

    ComPtr<IDWriteFontFileLoader> loader;
    hr = fontFile->GetLoader(&loader);
    if (FAILED(hr)) return result;

    ComPtr<IDWriteLocalFontFileLoader> localLoader;
    if (SUCCEEDED(loader.As(&localLoader)))
    {
        const void *key;
        UINT32      keySize;
        if (SUCCEEDED(fontFile->GetReferenceKey(&key, &keySize)))
        {
            UINT32 pathLen;
            hr = localLoader->GetFilePathLengthFromKey(key, keySize, &pathLen);
            if (SUCCEEDED(hr))
            {
                std::wstring filePath(pathLen, L'\0');
                hr = localLoader->GetFilePathFromKey(key, keySize, &filePath[0], pathLen + 1);
                if (SUCCEEDED(hr))
                {
                    return WCharUtils::ToString(filePath);
                }
            }
        }
    }
    return result;
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
}
}