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

constexpr auto MenuItem(const std::string_view &nameKey, const char* shortcut = nullptr, bool selected = false) -> bool
{
    Translation::Translate(nameKey.data(), g_widgetName);
    return ImGui::MenuItem(g_widgetName.c_str(), shortcut, selected);
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

    constexpr auto AllowOverlap() -> SelectableFlag &
    {
        flags |= ImGuiSelectableFlags_AllowOverlap;
        return *this;
    }

    constexpr auto AllowDoubleClick() -> SelectableFlag &
    {
        flags |= ImGuiSelectableFlags_AllowDoubleClick;
        return *this;
    }

    constexpr auto SpanAllColumns() -> SelectableFlag &
    {
        flags |= ImGuiSelectableFlags_SpanAllColumns;
        return *this;
    }
};

struct ChildFlag
{
    ImGuiChildFlags flags = ImGuiChildFlags_None;

    constexpr auto Borders() -> ChildFlag &
    {
        flags |= ImGuiChildFlags_Borders;
        return *this;
    }

    constexpr auto ResizeX() -> ChildFlag &
    {
        flags |= ImGuiChildFlags_ResizeX;
        return *this;
    }

    constexpr auto AutoResizeX() -> ChildFlag &
    {
        flags |= ImGuiChildFlags_AutoResizeX;
        return *this;
    }

    constexpr auto AutoResizeY() -> ChildFlag &
    {
        flags |= ImGuiChildFlags_AutoResizeY;
        return *this;
    }

    constexpr auto ResizeY() -> ChildFlag &
    {
        flags |= ImGuiChildFlags_ResizeY;
        return *this;
    }
};

struct WindowFlags
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;

    operator ImGuiWindowFlags() const
    {
        return flags;
    }

    void operator|=(const WindowFlags &other)
    {
        flags |= other.flags;
    }

    WindowFlags &None()
    {
        flags = ImGuiWindowFlags_None;
        return *this;
    }

    WindowFlags &NoTitleBar()
    {
        flags |= ImGuiWindowFlags_NoTitleBar;
        return *this;
    }

    WindowFlags &NoResize()
    {
        flags |= ImGuiWindowFlags_NoResize;
        return *this;
    }

    WindowFlags &NoMove()
    {
        flags |= ImGuiWindowFlags_NoMove;
        return *this;
    }

    WindowFlags &NoScrollbar()
    {
        flags |= ImGuiWindowFlags_NoScrollbar;
        return *this;
    }

    WindowFlags &NoScrollWithMouse()
    {
        flags |= ImGuiWindowFlags_NoScrollWithMouse;
        return *this;
    }

    WindowFlags &NoCollapse()
    {
        flags |= ImGuiWindowFlags_NoCollapse;
        return *this;
    }

    WindowFlags &AlwaysAutoResize()
    {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;
        return *this;
    }

    WindowFlags &NoBackground()
    {
        flags |= ImGuiWindowFlags_NoBackground;
        return *this;
    }

    WindowFlags &NoSavedSettings()
    {
        flags |= ImGuiWindowFlags_NoSavedSettings;
        return *this;
    }

    WindowFlags &NoMouseInputs()
    {
        flags |= ImGuiWindowFlags_NoMouseInputs;
        return *this;
    }

    WindowFlags &MenuBar()
    {
        flags |= ImGuiWindowFlags_MenuBar;
        return *this;
    }

    WindowFlags &HorizontalScrollbar()
    {
        flags |= ImGuiWindowFlags_HorizontalScrollbar;
        return *this;
    }

    WindowFlags &NoFocusOnAppearing()
    {
        flags |= ImGuiWindowFlags_NoFocusOnAppearing;
        return *this;
    }

    WindowFlags &NoBringToFrontOnFocus()
    {
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        return *this;
    }

    WindowFlags &AlwaysVerticalScrollbar()
    {
        flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
        return *this;
    }

    WindowFlags &AlwaysHorizontalScrollbar()
    {
        flags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
        return *this;
    }

    WindowFlags &AlwaysUseWindowPadding()
    {
        flags |= ImGuiWindowFlags_AlwaysUseWindowPadding;
        return *this;
    }

    WindowFlags &NoNavInputs()
    {
        flags |= ImGuiWindowFlags_NoNavInputs;
        return *this;
    }

    WindowFlags &NoNavFocus()
    {
        flags |= ImGuiWindowFlags_NoNavFocus;
        return *this;
    }

    WindowFlags &UnsavedDocument()
    {
        flags |= ImGuiWindowFlags_UnsavedDocument;
        return *this;
    }

    WindowFlags &NoDocking()
    {
        flags |= ImGuiWindowFlags_NoDocking;
        return *this;
    }

    WindowFlags &NoDecoration()
    {
        flags |= ImGuiWindowFlags_NoDecoration;
        return *this;
    }

    WindowFlags &NoNav()
    {
        flags |= ImGuiWindowFlags_NoNav;
        return *this;
    }
};

struct TabBarFlags
{
    ImGuiTabBarFlags flags = ImGuiTabBarFlags_None;

    constexpr auto Reorderable()
    {
        flags |= ImGuiTabBarFlags_Reorderable;
        return *this;
    }

    constexpr auto DrawSelectedOverline()
    {
        flags |= ImGuiTabBarFlags_DrawSelectedOverline;
        return *this;
    }

    constexpr auto AutoSelectNewTabs()
    {
        flags |= ImGuiTabBarFlags_AutoSelectNewTabs;
        return *this;
    }

    constexpr auto TabListPopupButton()
    {
        flags |= ImGuiTabBarFlags_TabListPopupButton;
        return *this;
    }

    constexpr auto NoCloseWithMiddleMouseButton()
    {
        flags |= ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
        return *this;
    }

    constexpr auto NoTabListScrollingButtons()
    {
        flags |= ImGuiTabBarFlags_NoTabListScrollingButtons;
        return *this;
    }

    constexpr auto NoTooltip()
    {
        flags |= ImGuiTabBarFlags_NoTooltip;
        return *this;
    }

    constexpr auto FittingPolicyResizeDown()
    {
        flags |= ImGuiTabBarFlags_FittingPolicyResizeDown;
        return *this;
    }

    constexpr auto FittingPolicyScroll()
    {
        flags |= ImGuiTabBarFlags_FittingPolicyScroll;
        return *this;
    }

    constexpr auto FittingPolicyMask()
    {
        flags |= ImGuiTabBarFlags_FittingPolicyMask_;
        return *this;
    }

    constexpr auto FittingPolicyDefault()
    {
        flags |= ImGuiTabBarFlags_FittingPolicyDefault_;
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