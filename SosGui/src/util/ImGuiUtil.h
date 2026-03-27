//
// Created by jamie on 2025/4/6.
//

#pragma once

#include "Translation.h"
#include "imgui.h"
#include "imguiex/ImGuiEx.h"

#include <array>
#include <string>

// ImGui v1.92

namespace SosGui::ImGuiUtil
{
static std::string       g_widgetName;
static constexpr ImColor RED_COLOR = ImColor(255, 0, 0, 255);

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

constexpr auto BeginTabItem(const std::string &nameKey, bool *open = nullptr, ImGuiTabItemFlags flags = 0) -> bool
{
    Translation::Translate(nameKey.c_str(), g_widgetName);
    return ImGui::BeginTabItem(g_widgetName.c_str(), open, flags);
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

template <typename ValueType>
constexpr auto Value(const char *label, ValueType value) -> void
{
    Translation::Translate(label, g_widgetName);
    ImGui::Value(g_widgetName.c_str(), value);
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

constexpr auto BeginChild(const char *windowId, const ImVec2 &size = ImVec2(0, 0), ImGuiChildFlags chiildFlags = ImGuiChildFlags_None) -> bool
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

void AddItemRectWithCol(ImGuiCol colorIndex, float thickness = 1.0F);

constexpr void AddItemRect(const ImColor color, const float thickness = 1.0F)
{
    auto *drawList = ImGui::GetWindowDrawList();
    drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color, 0, ImDrawFlags_None, thickness);
}

void may_update_table_sort_dir(bool &ascend);

constexpr void OpenPopup(std::string_view strId, ImGuiPopupFlags flags = ImGuiPopupFlags_None)
{
    const auto popupId = ImGui::GetID(ImGuiEx::TextStart(strId), ImGuiEx::TextEnd(strId));
    ImGui::OpenPopup(popupId, flags);
}

constexpr void Text(std::string_view text)
{
    ImGui::TextUnformatted(ImGuiEx::TextStart(text), ImGuiEx::TextEnd(text));
}

bool IconButton(std::string_view icon, const ImVec2 &size = {});

struct DebounceInput
{
    std::chrono::time_point<std::chrono::system_clock> prevEditTime;
    ImGuiTextFilter                                    filter;
    bool                                               dirty    = true;
    std::chrono::milliseconds                          duration = 300ms;

    explicit DebounceInput() : prevEditTime(std::chrono::system_clock::now()) {}

    virtual ~DebounceInput() = default;

    bool PassFilter(const char *text) const { return filter.PassFilter(text); }

    virtual void OnInput()
    {
        filter.Build();
        prevEditTime = std::chrono::system_clock::now();
        dirty        = true;
    }

    virtual bool Draw(const char *label, const char *hintText);

    virtual void Clear();
};

} // namespace SosGui::ImGuiUtil
