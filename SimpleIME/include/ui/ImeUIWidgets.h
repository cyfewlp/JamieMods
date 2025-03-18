//
// Created by jamie on 2025/3/7.
//

#ifndef IMEUIWIDGETS_H
#define IMEUIWIDGETS_H

#pragma once

#include "core/Translation.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"

#include <unordered_map>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImeWnd;

        class ImeUIWidgets
        {
            using String  = const char *;
            using OnClick = std::function<void()>;

        public:
            explicit ImeUIWidgets(Translation *translation) : m_translation(translation)
            {
            }

            static void RenderInputMethodChooseWidget(LangProfileUtil *pLangProfileUtil, const ImeWnd *pImeWnd);
            void        StateWidget(String label, const bool isEnabled) const;
            void        Button(String label, OnClick onClick) const;
            void        Checkbox(String label, bool &checked,
                                 const std::function<bool(bool keepImeOpen)> &&onChecked = {}) const;
            auto        ComboApply(String label, const std::vector<std::string> &values,
                                   std::function<bool(const std::string &)> onApply) -> uint32_t;

            constexpr void RadioButton(String label, int *pValue, int value)
            {
                const auto *name = m_translation->Get(label);
                ImGui::RadioButton(name, pValue, value);
                TrySetItemTooltip(label);
            }

            auto Begin(String windowName, bool *open, ImGuiWindowFlags flags) -> void;

            auto SetUInt32Var(String name, uint32_t value) -> void;
            auto GetBoolVar(String name) -> bool;
            auto TrySetItemTooltip(String label) const -> void;

        private:
            std::map<String, uint32_t> m_uiUint32Vars;
            std::map<String, bool>     m_uiBoolVars;
            Translation               *m_translation;
        };
    }
}

#endif // IMEUIWIDGETS_H
