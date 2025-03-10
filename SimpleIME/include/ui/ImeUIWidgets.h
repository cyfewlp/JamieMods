//
// Created by jamie on 2025/3/7.
//

#ifndef IMEUIWIDGETS_H
#define IMEUIWIDGETS_H

#pragma once

#include "ImeWnd.hpp"
#include "tsf/LangProfileUtil.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImeUIWidgets
        {
        public:
            static void RenderInputMethodChooseWidget(LangProfileUtil *pLangProfileUtil, const ImeWnd *pImeWnd);
            static void RenderEnableModWidget(std::function<bool(bool)> &&onEnable);
            static void RenderImeStateWidget(bool isDisabled);
            static void RenderImeFocusStateWidget(bool isFocused);
            static void RenderKeepImeOpenWidget(const std::function<bool(bool keepImeOpen)> &&onChecked);
            static void RenderThemeChooseWidget(const std::vector<std::string> &themeNames, uint32_t &selectedTheme,
                                                std::function<bool(const std::string &)> &&onApply);
        };
    }
}

#endif // IMEUIWIDGETS_H
