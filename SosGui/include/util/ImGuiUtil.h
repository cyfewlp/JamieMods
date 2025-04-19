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
        static std::string       g_widgetName;
        static constexpr ImColor RED_COLOR = ImColor(255, 0, 0, 255);

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

        static constexpr auto CheckBox(const std::string &name, bool *isChecked) -> bool
        {
            Translation::Translate(name.c_str(), g_widgetName);
            return ImGui::Checkbox(g_widgetName.c_str(), isChecked);
        }

        template <size_t Size>
        constexpr auto InputText(const char *name, std::array<char, Size> &inputBuf) -> bool
        {
            Translation::Translate(name, g_widgetName);
            return ImGui::InputText(g_widgetName.c_str(), inputBuf.data(), inputBuf.size());
        }

        constexpr bool BeginCombo(const char *nameKey, const char *previeValue)
        {
            Translation::Translate(nameKey, g_widgetName);
            return ImGui::BeginCombo(g_widgetName.c_str(), previeValue);
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

        template <typename String>
        constexpr auto SetItemTooltip(String &&content) -> void
        {
            Translation::Translate(content.c_str(), g_widgetName);
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

        constexpr bool SliderInt(const char *nameKey, int *curPageSize, int min, int max)
        {
            Translation::Translate(nameKey, g_widgetName);
            return ImGui::SliderInt(g_widgetName.c_str(), curPageSize, min, max);
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

        constexpr void AddItemRectWithCol(ImGuiCol colorIndex, float thickness = 1.0F)
        {
            auto *drawList = ImGui::GetWindowDrawList();
            auto color = ImGui::GetColorU32(colorIndex);
            drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color, 0, ImDrawFlags_None, thickness);
        }

        constexpr void AddItemRect(ImColor color, float thickness = 1.0F)
        {
            auto *drawList = ImGui::GetWindowDrawList();
            drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color, 0, ImDrawFlags_None, thickness);
        }

        struct SelectableFlag
        {
            ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;

            constexpr auto AllowOverlap() -> SelectableFlag &
            {
                flags |= ImGuiSelectableFlags_AllowOverlap;
                return *this;
            }

            constexpr auto SpanAllColumns() -> SelectableFlag &
            {
                flags |= ImGuiSelectableFlags_SpanAllColumns;
                return *this;
            }
        };

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

            explicit operator bool() const
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