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

        constexpr auto TextWrapped(const char *content) -> void
        {
            Translate(content, g_widgetName);
            ImGui::TextWrapped("%s", g_widgetName.c_str());
        }

        constexpr auto TextWrapped(const std::string &&content) -> void
        {
            Translate(content.c_str(), g_widgetName);
            ImGui::TextWrapped("%s", g_widgetName.c_str());
        }

        constexpr auto Selectable(const std::string &string, bool isSelected) -> bool
        {
            Translate(string.c_str(), g_widgetName);
            return ImGui::Selectable(g_widgetName.c_str(), isSelected);
        }

        constexpr bool BeginPopupModal(const char *name)
        {
            Translate(name, g_widgetName);
            return ImGui::BeginPopupModal(name);
        }

        constexpr void OpenPopup(const char *name)
        {
            Translate(name, g_widgetName);
            ImGui::OpenPopup(g_widgetName.c_str(), ImGuiPopupFlags_NoOpenOverExistingPopup);
        }

        template <size_t Columns>
        struct ImTable
        {
            std::string                      name;
            uint32_t                         rows;
            ImGuiTableFlags                  flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
            std::array<std::string, Columns> headersRow;

            ImTable() : rows(0), flags(0)
            {
            }

            ImTable(std::string_view name, uint32_t rows, std::array<std::string_view, Columns> headersRow)
                : name(name), rows(rows), headersRow(headersRow)
            {
            }
        };

        template <size_t Columns>
        bool RenderTable(ImTable<Columns> &imTable, std::function<bool(uint32_t &)> renderRow)
        {
            bool result = true;
            if (ImGui::BeginTable(imTable.name.c_str(), Columns, imTable.flags))
            {
                ImGui::PushID("HeaderRow");
                for (size_t idx = 0; idx < Columns; ++idx)
                {
                    ImGui::TableSetupColumn(imTable.headersRow[idx].c_str());
                }
                ImGui::TableHeadersRow();

                for (uint32_t rowIdx = 0; rowIdx < imTable.rows;)
                {
                    ImGui::PushID(rowIdx);
                    ImGui::TableNextRow();
                    if (!renderRow(rowIdx))
                    {
                        result = false;
                        break;
                    }
                    ImGui::PopID();
                }
                ImGui::PopID();
                ImGui::EndTable();
            }
            return result;
        }

        template <size_t Columns>
        constexpr void RenderTable(ImTable<Columns> &imTable, std::function<void(int)> renderRow)
        {
            if (ImGui::BeginTable(imTable.name.c_str(), Columns, imTable.flags))
            {
                ImGui::PushID("HeaderRow");
                for (size_t idx = 0; idx < Columns; ++idx)
                {
                    ImGui::TableSetupColumn(imTable.headersRow[idx].c_str());
                }
                ImGui::TableHeadersRow();

                for (uint32_t rowIdx = 0; rowIdx < imTable.rows; ++rowIdx)
                {
                    ImGui::PushID(rowIdx);
                    ImGui::TableNextRow();
                    renderRow(rowIdx);
                    ImGui::PopID();
                }
                ImGui::PopID();
                ImGui::EndTable();
            }
        }
    }
}

#endif // IMGUIUTIL_H
