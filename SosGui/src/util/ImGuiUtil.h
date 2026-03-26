//
// Created by jamie on 2025/4/6.
//

#pragma once

#include "Translation.h"
#include "imgui.h"

#include <array>
#include <string>

// ImGui v1.92

namespace SosGui::ImGuiUtil
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

constexpr auto BeginMenu(const char *nameKey) -> bool
{
    Translation::Translate(nameKey, g_widgetName);
    return ImGui::BeginMenu(g_widgetName.c_str());
}

constexpr auto MenuItem(const std::string_view &nameKey, const char *shortcut = nullptr, bool selected = false) -> bool
{
    Translation::Translate(nameKey.data(), g_widgetName);
    return ImGui::MenuItem(g_widgetName.c_str(), shortcut, selected);
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
