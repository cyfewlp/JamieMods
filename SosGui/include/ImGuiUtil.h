//
// Created by jamie on 2025/4/6.
//

#ifndef IMGUIUTIL_H
#define IMGUIUTIL_H

#pragma once

#include "SimpleIME/include/ImGuiThemeLoader.h"

#include <SKSE/Translation.h>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    namespace ImGuiUtil
    {
        static std::string g_widgetName;

        constexpr void Translate(const char *key, std::string &result)
        {
            static std::unordered_map<std::string, std::string> cache;
            if (cache.contains(key))
            {
                result = cache.at(key);
                return;
            }
            result.assign(key);
            SKSE::Translation::Translate(key, result);
            cache.emplace(key, result);
        }

        constexpr auto Translate(const char *key) -> std::string
        {
            std::string result;
            SKSE::Translation::Translate(key, result);
            return result;
        }

        constexpr auto Button(const char *name) -> bool
        {
            Translate(name, g_widgetName);
            return ImGui::Button(g_widgetName.c_str());
        }

        constexpr auto CheckBox(const char *name, bool *isChecked) -> bool
        {
            Translate(name, g_widgetName);
            return ImGui::Checkbox(g_widgetName.c_str(), isChecked);
        }

        template <size_t Size>
        constexpr auto InputText(const char *name, std::array<char, Size> &inputBuf) -> bool
        {
            Translate(name, g_widgetName);
            return ImGui::InputText(g_widgetName.c_str(), inputBuf.data(), inputBuf.size());
        }

        constexpr auto SetItemTooltip(const char *content) -> void
        {
            Translate(content, g_widgetName);
            ImGui::SetItemTooltip("%s", g_widgetName.c_str());
        }

        constexpr auto SeparatorText(const char *content) -> void
        {
            Translate(content, g_widgetName);
            ImGui::SeparatorText(g_widgetName.c_str());
        }

        constexpr auto Text(const char *content) -> void
        {
            Translate(content, g_widgetName);
            ImGui::Text("%s", g_widgetName.c_str());
        }

        constexpr auto TextScale(const char *content, float scale = 1.0f) -> void
        {
            Translate(content, g_widgetName);
            ImGui::PushFontSize(scale);
            ImGui::Text("%s", g_widgetName.c_str());
            ImGui::PopFontSize();
        }

        constexpr auto Text(const std::string &&content) -> void
        {
            Translate(content.c_str(), g_widgetName);
            ImGui::Text("%s", g_widgetName.c_str());
        }

        constexpr auto Selectable(const std::string &string, bool isSelected) -> bool
        {
            Translate(string.c_str(), g_widgetName);
            return ImGui::Selectable(g_widgetName.c_str(), isSelected);
        }
    }
}

#endif // IMGUIUTIL_H
