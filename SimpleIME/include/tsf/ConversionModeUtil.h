//
// Created by jamie on 2026/3/17.
//

#pragma once

#include "core/State.h"

#include <string>
#include <winnt.h>

namespace Ime
{
auto GetConversionModeNameShort(LANGID langId, Core::State::ConversionMode conversionMode, bool keyboardOpen) -> std::string
{
    conversionMode.Clear(Core::State::ConversionMode::Flags::ROMAN);
    if (PRIMARYLANGID(langId) == LANG_JAPANESE)
    {
        if (!keyboardOpen)
        {
            if (conversionMode.IsAlphanumeric())
            {
                return "ａ"; ///< 半角英数
            }
            if ((conversionMode.IsFullShape()))
            {
                return "Ａ"; ///< 全角英数
            }
        }
        else
        {
            if (conversionMode.IsAlphanumeric())
            {
                return "ａ"; ///< 半角英数
            }
            if (conversionMode.IsNative() && conversionMode.IsFullShape())
            {
                return "あ"; ///< ひらがな
            }
            if ((conversionMode.IsKatakana() && conversionMode.IsFullShape()))
            {
                return "カ"; ///< 全角カタカナ
            }
            if ((conversionMode.IsFullShape()))
            {
                return "Ａ"; ///< 全角英数
            }
            if ((conversionMode.IsNative() && conversionMode.IsKatakana()))
            {
                return "ｶ"; ///< 半角カタカナ
            }
        }
    }
    if (PRIMARYLANGID(langId) == LANG_CHINESE)
    {
        if (conversionMode.IsAlphanumeric()) return "英";

        const auto native    = conversionMode.IsNative();
        const auto fullShape = conversionMode.IsFullShape();
        if (native && fullShape) return "全角";
        if (fullShape) return "英(全角)";
        if (!native) return "英";
        return "中";
    }
    return "";
}
} // namespace Ime
