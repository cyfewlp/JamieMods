//
// Created by jamie on 2025/4/6.
//

#ifndef IMGUIUTIL_H
#define IMGUIUTIL_H

#pragma once

#include "Translation.h"
#include "imgui.h"

#include <array>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    namespace ImGuiUtil
    {
        static std::string g_widgetName;

        static constexpr auto Button(const char *name, const ImVec2 &size = ImVec2(0, 0)) -> bool
        {
            Translation::Translate(name, g_widgetName);
            return ImGui::Button(g_widgetName.c_str(), size);
        }

        static constexpr auto CheckBox(const char *name, bool *isChecked) -> bool
        {
            Translation::Translate(name, g_widgetName);
            return ImGui::Checkbox(g_widgetName.c_str(), isChecked);
        }

        template <size_t Size>
        constexpr auto InputText(const char *name, std::array<char, Size> &inputBuf) -> bool
        {
            Translation::Translate(name, g_widgetName);
            return ImGui::InputText(g_widgetName.c_str(), inputBuf.data(), inputBuf.size());
        }

        constexpr auto BeginTabItem(const std::string &nameKey) -> bool
        {
            Translation::Translate(nameKey.c_str(), g_widgetName);
            return ImGui::BeginTabItem(g_widgetName.c_str());
        }

        constexpr auto SetItemTooltip(const char *content) -> void
        {
            Translation::Translate(content, g_widgetName);
            ImGui::SetItemTooltip("%s", g_widgetName.c_str());
        }

        constexpr auto SeparatorText(const char *content) -> void
        {
            Translation::Translate(content, g_widgetName);
            ImGui::SeparatorText(g_widgetName.c_str());
        }

        constexpr auto Text(const char *content) -> void
        {
            Translation::Translate(content, g_widgetName);
            ImGui::Text("%s", g_widgetName.c_str());
        }

        constexpr auto TextScale(const char *content, float scale = 1.0f) -> void
        {
            Translation::Translate(content, g_widgetName);
            ImGui::PushFontSize(scale);
            ImGui::Text("%s", g_widgetName.c_str());
            ImGui::PopFontSize();
        }

        constexpr auto Text(const std::string &&content) -> void
        {
            Translation::Translate(content.c_str(), g_widgetName);
            ImGui::Text("%s", g_widgetName.c_str());
        }

        constexpr auto TextWrapped(const char *content) -> void
        {
            Translation::Translate(content, g_widgetName);
            ImGui::TextWrapped("%s", g_widgetName.c_str());
        }

        constexpr auto TextWrapped(const std::string &&content) -> void
        {
            Translation::Translate(content.c_str(), g_widgetName);
            ImGui::TextWrapped("%s", g_widgetName.c_str());
        }

        constexpr auto Selectable(const std::string &string, bool isSelected, ImGuiSelectableFlags flags = 0) -> bool
        {
            Translation::Translate(string.c_str(), g_widgetName);
            return ImGui::Selectable(g_widgetName.c_str(), isSelected, flags);
        }

        constexpr auto MenuItem(const std::string_view &nameKey) -> bool
        {
            Translation::Translate(nameKey.data(), g_widgetName);
            return ImGui::MenuItem(g_widgetName.c_str());
        }

        constexpr auto BeginChild(const char *windowId, const ImVec2 &size = ImVec2(0, 0),
                                  ImGuiChildFlags chiildFlags = ImGuiChildFlags_None) -> bool
        {
            Translation::Translate(windowId, g_widgetName);
            return ImGui::BeginChild(g_widgetName.c_str(), size, chiildFlags);
        }

        constexpr bool BeginPopupModal(const std::string &nameKey)
        {
            Translation::Translate(nameKey.c_str(), g_widgetName);
            return ImGui::BeginPopupModal(g_widgetName.c_str(), nullptr, ImGuiWindowFlags_None);
        }

        constexpr bool BeginPopupModal(const char *name)
        {
            Translation::Translate(name, g_widgetName);
            return ImGui::BeginPopupModal(g_widgetName.c_str(), nullptr, ImGuiWindowFlags_None);
        }

        constexpr void OpenPopup(const char *name)
        {
            Translation::Translate(name, g_widgetName);
            ImGui::OpenPopup(g_widgetName.c_str(), ImGuiPopupFlags_None);
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
        constexpr auto BeginTable(ImTable<Columns> &imTable)
        {
            return ImGui::BeginTable(imTable.name.c_str(), Columns, imTable.flags);
        }

        template <size_t Columns>
        constexpr auto TableHeadersRow(ImTable<Columns> &imTable)
        {
            ImGui::PushID("HeaderRow");
            for (size_t idx = 0; idx < Columns; ++idx)
            {
                ImGui::TableSetupColumn(imTable.headersRow[idx].c_str());
            }
            ImGui::TableHeadersRow();
            ImGui::PopID();
        }

        template <size_t Columns>
        bool RenderTable(ImTable<Columns> &imTable, std::function<bool(uint32_t &)> renderRow)
        {
            bool result = true;
            if (BeginTable(imTable))
            {
                TableHeadersRow(imTable);

                for (uint32_t rowIdx = 0; rowIdx < imTable.rows;)
                {
                    ImGui::PushID(rowIdx);
                    ImGui::TableNextRow();
                    if (!renderRow(rowIdx))
                    {
                        result = false;
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopID();
                }
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

        struct PushIdGuard
        {
            template <typename ID>
            PushIdGuard(ID id)
            {
                ImGui::PushID(id);
            }

            ~PushIdGuard()
            {
                ImGui::PopID();
            }
        };

        struct ChildGuard
        {
            bool isBegin;

            explicit ChildGuard(const char *name, const ImVec2 &size, ImGuiChildFlags flags = 0)
            {
                isBegin = ImGui::BeginChild(name, size, flags);
            }

            explicit operator bool()
            {
                return isBegin;
            }

            ~ChildGuard()
            {
                ImGui::EndChild();
            }
        };
    }
}

#endif // IMGUIUTIL_H
