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

// ImGui v1.92
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

auto TextScale(const char *content, float scale = 1.0f) -> void;

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

constexpr auto BeginChild(
    const char *windowId, const ImVec2 &size = ImVec2(0, 0), ImGuiChildFlags chiildFlags = ImGuiChildFlags_None
) -> bool
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

    bool PassFilter(const char *text) const
    {
        return filter.PassFilter(text);
    }

    virtual void OnInput()
    {
        filter.Build();
        prevEditTime = std::chrono::system_clock::now();
        dirty        = true;
    }

    virtual bool Draw(const char *label, const char *hintText);

    virtual void clear();
};

struct SelectableFlag
{
    ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;

    consteval auto AllowOverlap() -> SelectableFlag
    {
        return SelectableFlag{flags |= ImGuiSelectableFlags_AllowOverlap};
    }

    consteval auto AllowDoubleClick() -> SelectableFlag
    {
        return SelectableFlag{flags |= ImGuiSelectableFlags_AllowDoubleClick};
    }

    consteval auto SpanAllColumns() -> SelectableFlag
    {
        return SelectableFlag{flags |= ImGuiSelectableFlags_SpanAllColumns};
    }
};

struct InputTextFlags
{
    ImGuiInputFlags flags = ImGuiInputFlags_None;

    consteval operator ImGuiInputFlags() const
    {
        return flags;
    }

    consteval auto None() const
    {
        return InputTextFlags{ImGuiInputTextFlags_None};
    }

    consteval auto CharsDecimal() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsDecimal};
    }

    consteval auto CharsHexadecimal() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsHexadecimal};
    }

    consteval auto CharsScientific() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsScientific};
    }

    consteval auto CharsUppercase() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsUppercase};
    }

    consteval auto CharsNoBlank() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsNoBlank};
    }

    consteval auto AllowTabInput() const
    {
        return InputTextFlags{ImGuiInputTextFlags_AllowTabInput};
    }

    consteval auto EnterReturnsTrue() const
    {
        return InputTextFlags{ImGuiInputTextFlags_EnterReturnsTrue};
    }

    consteval auto EscapeClearsAll() const
    {
        return InputTextFlags{ImGuiInputTextFlags_EscapeClearsAll};
    }

    consteval auto CtrlEnterForNewLine() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CtrlEnterForNewLine};
    }

    consteval auto ReadOnly() const
    {
        return InputTextFlags{ImGuiInputTextFlags_ReadOnly};
    }

    consteval auto Password() const
    {
        return InputTextFlags{ImGuiInputTextFlags_Password};
    }

    consteval auto AlwaysOverwrite() const
    {
        return InputTextFlags{ImGuiInputTextFlags_AlwaysOverwrite};
    }

    consteval auto AutoSelectAll() const
    {
        return InputTextFlags{ImGuiInputTextFlags_AutoSelectAll};
    }

    consteval auto ParseEmptyRefVal() const
    {
        return InputTextFlags{ImGuiInputTextFlags_ParseEmptyRefVal};
    }

    consteval auto DisplayEmptyRefVal() const
    {
        return InputTextFlags{ImGuiInputTextFlags_DisplayEmptyRefVal};
    }

    consteval auto NoHorizontalScroll() const
    {
        return InputTextFlags{ImGuiInputTextFlags_NoHorizontalScroll};
    }

    consteval auto NoUndoRedo() const
    {
        return InputTextFlags{ImGuiInputTextFlags_NoUndoRedo};
    }

    consteval auto ElideLeft() const
    {
        return InputTextFlags{ImGuiInputTextFlags_ElideLeft};
    }

    consteval auto CallbackCompletion() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackCompletion};
    }

    consteval auto CallbackHistory() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackHistory};
    }

    consteval auto CallbackAlways() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackAlways};
    }

    consteval auto CallbackCharFilter() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackCharFilter};
    }

    consteval auto CallbackResize() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackResize};
    }

    consteval auto CallbackEdit() const
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackEdit};
    }
};

struct ChildFlag
{
    ImGuiChildFlags flags = ImGuiChildFlags_None;

    consteval operator ImGuiChildFlags() const
    {
        return flags;
    }

    consteval auto Borders() -> ChildFlag
    {
        return ChildFlag{flags |= ImGuiChildFlags_Borders};
    }

    consteval auto ResizeX() -> ChildFlag
    {
        return ChildFlag{flags |= ImGuiChildFlags_ResizeX};
    }

    consteval auto AutoResizeX() -> ChildFlag
    {
        return ChildFlag{flags |= ImGuiChildFlags_AutoResizeX};
    }

    consteval auto AutoResizeY() -> ChildFlag
    {
        return ChildFlag{flags |= ImGuiChildFlags_AutoResizeY};
    }

    consteval auto ResizeY() -> ChildFlag
    {
        return ChildFlag{flags |= ImGuiChildFlags_ResizeY};
    }
};

struct WindowFlags
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;

    constexpr operator ImGuiWindowFlags() const
    {
        return flags;
    }

    void operator|=(const WindowFlags &other)
    {
        flags |= other.flags;
    }

    consteval WindowFlags None()
    {
        return WindowFlags{flags = ImGuiWindowFlags_None};
    }

    consteval WindowFlags NoTitleBar()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoTitleBar};
    }

    consteval WindowFlags NoResize()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoResize};
    }

    consteval WindowFlags NoMove()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoMove};
    }

    consteval WindowFlags NoScrollbar()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoScrollbar};
    }

    consteval WindowFlags NoScrollWithMouse()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoScrollWithMouse};
    }

    consteval WindowFlags NoCollapse()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoCollapse};
    }

    consteval WindowFlags AlwaysAutoResize()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_AlwaysAutoResize};
    }

    consteval WindowFlags NoBackground()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoBackground};
    }

    consteval WindowFlags NoSavedSettings()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoSavedSettings};
    }

    consteval WindowFlags NoMouseInputs()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoMouseInputs};
    }

    consteval WindowFlags MenuBar()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_MenuBar};
    }

    consteval WindowFlags HorizontalScrollbar()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_HorizontalScrollbar};
    }

    consteval WindowFlags NoFocusOnAppearing()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoFocusOnAppearing};
    }

    consteval WindowFlags NoBringToFrontOnFocus()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoBringToFrontOnFocus};
    }

    consteval WindowFlags AlwaysVerticalScrollbar()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar};
    }

    consteval WindowFlags AlwaysHorizontalScrollbar()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar};
    }

    consteval WindowFlags AlwaysUseWindowPadding()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_AlwaysUseWindowPadding};
    }

    consteval WindowFlags NoNavInputs()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoNavInputs};
    }

    consteval WindowFlags NoNavFocus()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoNavFocus};
    }

    consteval WindowFlags UnsavedDocument()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_UnsavedDocument};
    }

    consteval WindowFlags NoDocking()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoDocking};
    }

    consteval WindowFlags NoDecoration()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoDecoration};
    }

    consteval WindowFlags NoNav()
    {
        return WindowFlags{flags |= ImGuiWindowFlags_NoNav};
    }
};

struct TabBarFlags
{
    ImGuiTabBarFlags flags = ImGuiTabBarFlags_None;

    consteval auto Reorderable()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_Reorderable};
    }

    consteval auto DrawSelectedOverline()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_DrawSelectedOverline};
    }

    consteval auto AutoSelectNewTabs()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_AutoSelectNewTabs};
    }

    consteval auto TabListPopupButton()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_TabListPopupButton};
    }

    consteval auto NoCloseWithMiddleMouseButton()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_NoCloseWithMiddleMouseButton};
    }

    consteval auto NoTabListScrollingButtons()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_NoTabListScrollingButtons};
    }

    consteval auto NoTooltip()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_NoTooltip};
    }

    consteval auto FittingPolicyResizeDown()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_FittingPolicyResizeDown};
    }

    consteval auto FittingPolicyScroll()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_FittingPolicyScroll};
    }

    consteval auto FittingPolicyMask()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_FittingPolicyMask_};
    }

    consteval auto FittingPolicyDefault()
    {
        return TabBarFlags{flags |= ImGuiTabBarFlags_FittingPolicyDefault_};
    }
};

struct ConfigFlags
{
    ImGuiConfigFlags flags = ImGuiConfigFlags_None;

    constexpr operator ImGuiConfigFlags() const
    {
        return flags;
    }

    constexpr ConfigFlags operator|(const ConfigFlags &other) const
    {
        return ConfigFlags{(flags | other.flags)};
    }

    constexpr ConfigFlags &operator|=(const ConfigFlags &other)
    {
        flags = (flags | other.flags);
        return *this;
    }

    consteval auto NavEnableKeyboard() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_NavEnableKeyboard};
    }

    consteval auto NavEnableGamepad() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_NavEnableGamepad};
    }

    consteval auto NoMouse() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_NoMouse};
    }

    consteval auto NoMouseCursorChange() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_NoMouseCursorChange};
    }

    consteval auto NoKeyboard() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_NoKeyboard};
    }

    consteval auto DockingEnable() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_DockingEnable};
    }

    consteval auto ViewportsEnable() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_ViewportsEnable};
    }

    consteval auto DpiEnableScaleViewports() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_DpiEnableScaleViewports};
    }

    consteval auto DpiEnableScaleFonts() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_DpiEnableScaleFonts};
    }

    consteval auto IsSRGB() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_IsSRGB};
    }

    consteval auto IsTouchScreen() const
    {
        return ConfigFlags{flags | ImGuiConfigFlags_IsTouchScreen};
    }
};
}
}

#endif // IMGUIUTIL_H