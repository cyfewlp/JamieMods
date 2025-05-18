//
// Created by jamie on 2025/5/12.
//

#ifndef IMGUIFLAGS_H
#define IMGUIFLAGS_H

#include "common/config.h"
#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiUtil
{

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

    static consteval auto None()
    {
        return InputTextFlags{ImGuiInputTextFlags_None};
    }

    static consteval auto CharsDecimal()
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsDecimal};
    }

    static consteval auto CharsHexadecimal()
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsHexadecimal};
    }

    static consteval auto CharsScientific()
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsScientific};
    }

    static consteval auto CharsUppercase()
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsUppercase};
    }

    static consteval auto CharsNoBlank()
    {
        return InputTextFlags{ImGuiInputTextFlags_CharsNoBlank};
    }

    static consteval auto AllowTabInput()
    {
        return InputTextFlags{ImGuiInputTextFlags_AllowTabInput};
    }

    static consteval auto EnterReturnsTrue()
    {
        return InputTextFlags{ImGuiInputTextFlags_EnterReturnsTrue};
    }

    static consteval auto EscapeClearsAll()
    {
        return InputTextFlags{ImGuiInputTextFlags_EscapeClearsAll};
    }

    static consteval auto CtrlEnterForNewLine()
    {
        return InputTextFlags{ImGuiInputTextFlags_CtrlEnterForNewLine};
    }

    static consteval auto ReadOnly()
    {
        return InputTextFlags{ImGuiInputTextFlags_ReadOnly};
    }

    static consteval auto Password()
    {
        return InputTextFlags{ImGuiInputTextFlags_Password};
    }

    static consteval auto AlwaysOverwrite()
    {
        return InputTextFlags{ImGuiInputTextFlags_AlwaysOverwrite};
    }

    static consteval auto AutoSelectAll()
    {
        return InputTextFlags{ImGuiInputTextFlags_AutoSelectAll};
    }

    static consteval auto ParseEmptyRefVal()
    {
        return InputTextFlags{ImGuiInputTextFlags_ParseEmptyRefVal};
    }

    static consteval auto DisplayEmptyRefVal()
    {
        return InputTextFlags{ImGuiInputTextFlags_DisplayEmptyRefVal};
    }

    static consteval auto NoHorizontalScroll()
    {
        return InputTextFlags{ImGuiInputTextFlags_NoHorizontalScroll};
    }

    static consteval auto NoUndoRedo()
    {
        return InputTextFlags{ImGuiInputTextFlags_NoUndoRedo};
    }

    static consteval auto ElideLeft()
    {
        return InputTextFlags{ImGuiInputTextFlags_ElideLeft};
    }

    static consteval auto CallbackCompletion()
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackCompletion};
    }

    static consteval auto CallbackHistory()
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackHistory};
    }

    static consteval auto CallbackAlways()
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackAlways};
    }

    static consteval auto CallbackCharFilter()
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackCharFilter};
    }

    static consteval auto CallbackResize()
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackResize};
    }

    static consteval auto CallbackEdit()
    {
        return InputTextFlags{ImGuiInputTextFlags_CallbackEdit};
    }
};

struct TableFlags
{
    ImGuiTableFlags flags = ImGuiTableFlags_None;

    consteval auto RowBg() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_RowBg};
    }

    consteval auto BordersInnerH() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_BordersInnerH};
    }

    consteval auto ContextMenuInBody() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_ContextMenuInBody};
    }

    consteval auto Borders() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_Borders};
    }

    consteval auto Sortable() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_Sortable};
    }

    consteval auto Hideable() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_Hideable};
    }

    consteval auto Reorderable() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_Reorderable};
    }

    consteval auto Resizable() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_Resizable};
    }

    consteval auto NoHostExtendX() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_NoHostExtendX};
    }

    consteval auto NoHostExtendY() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_NoHostExtendY};
    }

    consteval auto SizingStretchProp() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_SizingStretchProp};
    }

    consteval auto SizingFixedFit() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_SizingFixedFit};
    }

    consteval auto ScrollX() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_ScrollX};
    }

    consteval auto ScrollY() -> TableFlags
    {
        return TableFlags{flags |= ImGuiTableFlags_ScrollY};
    }
};

struct TableColumnFlags
{
    ImGuiTableColumnFlags flags = ImGuiTableColumnFlags_None;

    constexpr operator ImGuiTableColumnFlags() const noexcept
    {
        return flags;
    }

    consteval auto None() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_None};
    }

    consteval auto Disabled() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_Disabled};
    }

    consteval auto DefaultHide() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_DefaultHide};
    }

    consteval auto DefaultSort() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_DefaultSort};
    }

    consteval auto WidthStretch() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_WidthStretch};
    }

    consteval auto WidthFixed() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_WidthFixed};
    }

    consteval auto NoResize() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoResize};
    }

    consteval auto NoReorder() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoReorder};
    }

    consteval auto NoHide() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoHide};
    }

    consteval auto NoClip() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoClip};
    }

    consteval auto NoSort() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoSort};
    }

    consteval auto NoSortAscending() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoSortAscending};
    }

    consteval auto NoSortDescending() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoSortDescending};
    }

    consteval auto NoHeaderLabel() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoHeaderLabel};
    }

    consteval auto NoHeaderWidth() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_NoHeaderWidth};
    }

    consteval auto PreferSortAscending() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_PreferSortAscending};
    }

    consteval auto PreferSortDescending() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_PreferSortDescending};
    }

    consteval auto IndentEnable() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_IndentEnable};
    }

    consteval auto IndentDisable() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_IndentDisable};
    }

    consteval auto AngledHeader() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_AngledHeader};
    }

    consteval auto IsEnabled() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_IsEnabled};
    }

    consteval auto IsVisible() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_IsVisible};
    }

    consteval auto IsSorted() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_IsSorted};
    }

    consteval auto IsHovered() const -> TableColumnFlags
    {
        return TableColumnFlags{flags | ImGuiTableColumnFlags_IsHovered};
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

struct ComboFlags
{
    ImGuiComboFlags flags = ImGuiComboFlags_None;

    constexpr operator ImGuiComboFlags() const
    {
        return flags;
    }

    consteval auto None() const
    {
        return ComboFlags{flags | ImGuiComboFlags_None};
    }

    consteval auto PopupAlignLeft() const
    {
        return ComboFlags{flags | ImGuiComboFlags_PopupAlignLeft};
    }

    consteval auto HeightSmall() const
    {
        return ComboFlags{flags | ImGuiComboFlags_HeightSmall};
    }

    consteval auto HeightRegular() const
    {
        return ComboFlags{flags | ImGuiComboFlags_HeightRegular};
    }

    consteval auto HeightLarge() const
    {
        return ComboFlags{flags | ImGuiComboFlags_HeightLarge};
    }

    consteval auto HeightLargest() const
    {
        return ComboFlags{flags | ImGuiComboFlags_HeightLargest};
    }

    consteval auto NoArrowButton() const
    {
        return ComboFlags{flags | ImGuiComboFlags_NoArrowButton};
    }

    consteval auto NoPreview() const
    {
        return ComboFlags{flags | ImGuiComboFlags_NoPreview};
    }

    consteval auto WidthFitPreview() const
    {
        return ComboFlags{flags | ImGuiComboFlags_WidthFitPreview};
    }
};
}
}

#endif // IMGUIFLAGS_H
