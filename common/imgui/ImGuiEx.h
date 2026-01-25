//
// Created by jamie on 2026/1/19.
//

#pragma once

#include "common/config.h"

#include <imgui.h>

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx
{

class FontScope
{
    bool Pushed = false;

public:
    FontScope() = default;

    explicit FontScope(ImFont *font, float fontSize = 0.f) : Pushed(true)
    {
        ImGui::PushFont(font, fontSize);
    }

    FontScope(const FontScope &other)      = delete;
    FontScope &operator=(FontScope &other) = delete;

    FontScope &operator=(FontScope &&other) noexcept
    {
        Pushed       = other.Pushed;
        other.Pushed = false;
        return *this;
    }

    FontScope(FontScope &&other) noexcept : Pushed(other.Pushed)
    {
        other.Pushed = false;
    }

    ~FontScope()
    {
        if (Pushed)
        {
            ImGui::PopFont();
        }
    }
};

struct ColorHolder
{
    ImGuiCol idx;
    ImVec4   val;

    explicit constexpr ColorHolder(ImGuiCol idx, const ImVec4 &val) : idx(idx), val(val) {}

    // clang-format off

    static constexpr auto Text(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_Text,val};}
    static constexpr auto TextDisabled(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TextDisabled, val};}
    static constexpr auto WindowBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_WindowBg, val};}
    static constexpr auto ChildBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ChildBg, val};}
    static constexpr auto PopupBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_PopupBg, val};}
    static constexpr auto Border(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_Border, val};}
    static constexpr auto BorderShadow(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_BorderShadow, val};}
    static constexpr auto FrameBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_FrameBg, val};}
    static constexpr auto FrameBgHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_FrameBgHovered, val};}
    static constexpr auto FrameBgActive(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_FrameBgActive, val};}
    static constexpr auto TitleBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TitleBg, val};}
    static constexpr auto TitleBgActive(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TitleBgActive, val};}
    static constexpr auto TitleBgCollapsed(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TitleBgCollapsed, val};}
    static constexpr auto MenuBarBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_MenuBarBg, val};}
    static constexpr auto ScrollbarBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ScrollbarBg, val};}
    static constexpr auto ScrollbarGrab(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ScrollbarGrab, val};}
    static constexpr auto ScrollbarGrabHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ScrollbarGrabHovered, val};}
    static constexpr auto ScrollbarGrabActive(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ScrollbarGrabActive, val};}
    static constexpr auto CheckMark(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_CheckMark, val};}
    static constexpr auto SliderGrab(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_SliderGrab, val};}
    static constexpr auto SliderGrabActive(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_SliderGrabActive, val};}
    static constexpr auto Button(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_Button, val};}
    static constexpr auto ButtonHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ButtonHovered, val};}
    static constexpr auto ButtonActive(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ButtonActive, val};}
    static constexpr auto Header(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_Header, val};}
    static constexpr auto HeaderHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_HeaderHovered, val};}
    static constexpr auto HeaderActive(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_HeaderActive, val};}
    static constexpr auto Separator(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_Separator, val};}
    static constexpr auto SeparatorHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_SeparatorHovered, val};}
    static constexpr auto SeparatorActive(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_SeparatorActive, val};}
    static constexpr auto ResizeGrip(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ResizeGrip, val};}
    static constexpr auto ResizeGripHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ResizeGripHovered, val};}
    static constexpr auto ResizeGripActive(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ResizeGripActive, val};}
    static constexpr auto InputTextCursor(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_InputTextCursor, val};}
    static constexpr auto TabHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TabHovered, val};}
    static constexpr auto Tab(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_Tab, val};}
    static constexpr auto TabSelected(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TabSelected, val};}
    static constexpr auto TabSelectedOverline(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TabSelectedOverline, val};}
    static constexpr auto TabDimmed(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TabDimmed, val};}
    static constexpr auto TabDimmedSelected(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TabDimmedSelected, val};}
    static constexpr auto TabDimmedSelectedOverline(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TabDimmedSelectedOverline, val};}
    static constexpr auto PlotLines(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_PlotLines, val};}
    static constexpr auto PlotLinesHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_PlotLinesHovered, val};}
    static constexpr auto PlotHistogram(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_PlotHistogram, val};}
    static constexpr auto PlotHistogramHovered(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_PlotHistogramHovered, val};}
    static constexpr auto TableHeaderBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TableHeaderBg, val};}
    static constexpr auto TableBorderStrong(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TableBorderStrong, val};}
    static constexpr auto TableBorderLight(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TableBorderLight, val};}
    static constexpr auto TableRowBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TableRowBg, val};}
    static constexpr auto TableRowBgAlt(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TableRowBgAlt, val};}
    static constexpr auto TextLink(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TextLink, val};}
    static constexpr auto TextSelectedBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TextSelectedBg, val};}
    static constexpr auto TreeLines(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_TreeLines, val};}
    static constexpr auto DragDropTarget(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_DragDropTarget, val};}
    static constexpr auto DragDropTargetBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_DragDropTargetBg, val};}
    static constexpr auto UnsavedMarker(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_UnsavedMarker, val};}
    static constexpr auto NavCursor(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_NavCursor, val};}
    static constexpr auto NavWindowingHighlight(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_NavWindowingHighlight, val};}
    static constexpr auto NavWindowingDimBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_NavWindowingDimBg, val};}
    static constexpr auto ModalWindowDimBg(const ImVec4&val) -> ColorHolder {return ColorHolder{ImGuiCol_ModalWindowDimBg, val};}

    // clang-format on
};

struct StyleHolder
{
    enum class Type
    {
        Float,
        Vec2
    };

    ImGuiStyleVar idx;
    Type          type;
    ImVec2        val;

    explicit constexpr StyleHolder(const ImGuiStyleVar &idx, const Type &type, const ImVec2 &val)
        : idx(idx), type(type), val(val)
    {
    }

    // clang-format off
    static constexpr auto Alpha(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_Alpha, Type::Float, ImVec2(val, 0)};}
    static constexpr auto DisabledAlpha(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_DisabledAlpha, Type::Float, ImVec2(val, 0)};}
    static constexpr auto WindowPadding(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_WindowPadding, Type::Vec2, val};}
    static constexpr auto WindowRounding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_WindowRounding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto WindowBorderSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_WindowBorderSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto WindowMinSize(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_WindowMinSize, Type::Vec2, val};}
    static constexpr auto WindowTitleAlign(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_WindowTitleAlign, Type::Vec2, val};}
    static constexpr auto ChildRounding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ChildRounding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto ChildBorderSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ChildBorderSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto PopupRounding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_PopupRounding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto PopupBorderSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_PopupBorderSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto FramePadding(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_FramePadding, Type::Vec2, val};}
    static constexpr auto FrameRounding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_FrameRounding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto FrameBorderSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_FrameBorderSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto ItemSpacing(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ItemSpacing, Type::Vec2, val};}
    static constexpr auto ItemInnerSpacing(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ItemInnerSpacing, Type::Vec2, val};}
    static constexpr auto IndentSpacing(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_IndentSpacing, Type::Float, ImVec2(val, 0)};}
    static constexpr auto CellPadding(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_CellPadding, Type::Vec2, val};}
    static constexpr auto ScrollbarSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ScrollbarSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto ScrollbarRounding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ScrollbarRounding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto ScrollbarPadding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ScrollbarPadding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto GrabMinSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_GrabMinSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto GrabRounding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_GrabRounding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto ImageBorderSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ImageBorderSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TabRounding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TabRounding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TabBorderSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TabBorderSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TabMinWidthBase(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TabMinWidthBase, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TabMinWidthShrink(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TabMinWidthShrink, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TabBarBorderSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TabBarBorderSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TabBarOverlineSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TabBarOverlineSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TableAngledHeadersAngle(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TableAngledHeadersAngle, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TableAngledHeadersTextAlign(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TableAngledHeadersTextAlign, Type::Vec2, val};}
    static constexpr auto TreeLinesSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TreeLinesSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto TreeLinesRounding(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_TreeLinesRounding, Type::Float, ImVec2(val, 0)};}
    static constexpr auto ButtonTextAlign(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_ButtonTextAlign, Type::Vec2, val};}
    static constexpr auto SelectableTextAlign(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_SelectableTextAlign, Type::Vec2, val};}
    static constexpr auto SeparatorTextBorderSize(float val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_SeparatorTextBorderSize, Type::Float, ImVec2(val, 0)};}
    static constexpr auto SeparatorTextAlign(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_SeparatorTextAlign, Type::Vec2, val};}
    static constexpr auto SeparatorTextPadding(const ImVec2 &val) -> StyleHolder{return StyleHolder{ImGuiStyleVar_SeparatorTextPadding, Type::Vec2, val};}

    // clang-format on
};

class StyleGuard
{
    int varCount   = 0;
    int colorCount = 0;

public:
    StyleGuard() = default;

    StyleGuard(const StyleGuard &)            = delete;
    StyleGuard &operator=(const StyleGuard &) = delete;

    StyleGuard(StyleGuard &&other) noexcept
        : varCount(std::exchange(other.varCount, 0)), colorCount(std::exchange(other.colorCount, 0))
    {
    }

    StyleGuard &operator=(StyleGuard &&other) noexcept
    {
        if (this != &other)
        {
            this->Pop();
            varCount   = std::exchange(other.varCount, 0);
            colorCount = std::exchange(other.colorCount, 0);
        }
        return *this;
    }

    ~StyleGuard()
    {
        Pop();
    }

    constexpr auto Push(StyleHolder style) -> StyleGuard &
    {
        if (style.type == StyleHolder::Type::Float)
        {
            ImGui::PushStyleVar(style.idx, style.val.x);
        }
        else
        {
            ImGui::PushStyleVar(style.idx, style.val);
        }
        varCount++;
        return *this;
    }

    constexpr auto Push(ColorHolder s) -> StyleGuard &
    {
        ImGui::PushStyleColor(s.idx, s.val);
        colorCount++;
        return *this;
    }

    template <typename T>
    auto PushVar(ImGuiStyleVar idx, T val) -> StyleGuard &
    {
        ImGui::PushStyleVar(idx, val);
        varCount++;
        return *this;
    }

    auto PushColor(ImGuiCol idx, ImU32 col) -> StyleGuard &
    {
        ImGui::PushStyleColor(idx, col);
        colorCount++;
        return *this;
    }

    void Pop()
    {
        if (varCount > 0)
        {
            ImGui::PopStyleVar(varCount);
            varCount = 0;
        }
        if (colorCount > 0)
        {
            ImGui::PopStyleColor(colorCount);
            colorCount = 0;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
/// Fluent wrappers for ImGui flags designed for optimized developer experience.
/// These utilities leverage IDE Intellisense and concise method chaining to simplify
/// flag construction. All builders are marked 'constexpr', ensuring that the bitwise logic
/// is evaluated at compile-time, resulting in zero runtime overhead compared to manual
/// bitwise OR operations.
/////////////////////////////////////////////////////////////////////////////////////////////

#define CONFIG_COLOR(colorName) ColorConvert(colorsNode[#colorName].value_or(""), style.Colors[ImGuiCol_##colorName])

#define FLAGS_CLASS_BODY(className)                                                                                    \
    class className                                                                                                    \
    {                                                                                                                  \
        ImGui##className flags = 0;                                                                                    \
                                                                                                                       \
    public:                                                                                                            \
        constexpr className() = default;                                                                               \
        constexpr explicit className(ImGui##className f) : flags(f) {}

#define FLAGS_CLASS_FUNCTION(className, flagName)                                                                      \
    constexpr auto flagName() -> className &                                                                           \
    {                                                                                                                  \
        flags |= ImGui##className##_##flagName;                                                                        \
        return *this;                                                                                                  \
    }

#define FLAGS_CLASS_END(className)                                                                                     \
    constexpr operator ImGui##className() const                                                                        \
    {                                                                                                                  \
        return flags;                                                                                                  \
    }                                                                                                                  \
    }                                                                                                                  \
    ;

FLAGS_CLASS_BODY(WindowFlags);
FLAGS_CLASS_FUNCTION(WindowFlags, None)
FLAGS_CLASS_FUNCTION(WindowFlags, NoTitleBar)
FLAGS_CLASS_FUNCTION(WindowFlags, NoResize)
FLAGS_CLASS_FUNCTION(WindowFlags, NoMove)
FLAGS_CLASS_FUNCTION(WindowFlags, NoScrollbar)
FLAGS_CLASS_FUNCTION(WindowFlags, NoScrollWithMouse)
FLAGS_CLASS_FUNCTION(WindowFlags, NoCollapse)
FLAGS_CLASS_FUNCTION(WindowFlags, AlwaysAutoResize)
FLAGS_CLASS_FUNCTION(WindowFlags, NoBackground)
FLAGS_CLASS_FUNCTION(WindowFlags, NoSavedSettings)
FLAGS_CLASS_FUNCTION(WindowFlags, NoMouseInputs)
FLAGS_CLASS_FUNCTION(WindowFlags, MenuBar)
FLAGS_CLASS_FUNCTION(WindowFlags, HorizontalScrollbar)
FLAGS_CLASS_FUNCTION(WindowFlags, NoFocusOnAppearing)
FLAGS_CLASS_FUNCTION(WindowFlags, NoBringToFrontOnFocus)
FLAGS_CLASS_FUNCTION(WindowFlags, AlwaysVerticalScrollbar)
FLAGS_CLASS_FUNCTION(WindowFlags, AlwaysHorizontalScrollbar)
FLAGS_CLASS_FUNCTION(WindowFlags, NoNavInputs)
FLAGS_CLASS_FUNCTION(WindowFlags, NoNavFocus)
FLAGS_CLASS_FUNCTION(WindowFlags, UnsavedDocument)
FLAGS_CLASS_FUNCTION(WindowFlags, NoNav)
FLAGS_CLASS_FUNCTION(WindowFlags, NoDecoration)
FLAGS_CLASS_FUNCTION(WindowFlags, NoInputs)
FLAGS_CLASS_END(WindowFlags)

FLAGS_CLASS_BODY(ChildFlags);
FLAGS_CLASS_FUNCTION(ChildFlags, None)
FLAGS_CLASS_FUNCTION(ChildFlags, Borders)
FLAGS_CLASS_FUNCTION(ChildFlags, AlwaysUseWindowPadding)
FLAGS_CLASS_FUNCTION(ChildFlags, ResizeX)
FLAGS_CLASS_FUNCTION(ChildFlags, ResizeY)
FLAGS_CLASS_FUNCTION(ChildFlags, AutoResizeX)
FLAGS_CLASS_FUNCTION(ChildFlags, AutoResizeY)
FLAGS_CLASS_FUNCTION(ChildFlags, AlwaysAutoResize)
FLAGS_CLASS_FUNCTION(ChildFlags, FrameStyle)
FLAGS_CLASS_FUNCTION(ChildFlags, NavFlattened)
FLAGS_CLASS_END(ChildFlags)

FLAGS_CLASS_BODY(TableFlags);
FLAGS_CLASS_FUNCTION(TableFlags, None)
FLAGS_CLASS_FUNCTION(TableFlags, Resizable)
FLAGS_CLASS_FUNCTION(TableFlags, Reorderable)
FLAGS_CLASS_FUNCTION(TableFlags, Hideable)
FLAGS_CLASS_FUNCTION(TableFlags, Sortable)
FLAGS_CLASS_FUNCTION(TableFlags, NoSavedSettings)
FLAGS_CLASS_FUNCTION(TableFlags, ContextMenuInBody)
FLAGS_CLASS_FUNCTION(TableFlags, RowBg)
FLAGS_CLASS_FUNCTION(TableFlags, BordersInnerH)
FLAGS_CLASS_FUNCTION(TableFlags, BordersOuterH)
FLAGS_CLASS_FUNCTION(TableFlags, BordersInnerV)
FLAGS_CLASS_FUNCTION(TableFlags, BordersOuterV)
FLAGS_CLASS_FUNCTION(TableFlags, BordersH)
FLAGS_CLASS_FUNCTION(TableFlags, BordersV)
FLAGS_CLASS_FUNCTION(TableFlags, BordersInner)
FLAGS_CLASS_FUNCTION(TableFlags, BordersOuter)
FLAGS_CLASS_FUNCTION(TableFlags, Borders)
FLAGS_CLASS_FUNCTION(TableFlags, NoBordersInBody)
FLAGS_CLASS_FUNCTION(TableFlags, NoBordersInBodyUntilResize)
FLAGS_CLASS_FUNCTION(TableFlags, SizingFixedFit)
FLAGS_CLASS_FUNCTION(TableFlags, SizingFixedSame)
FLAGS_CLASS_FUNCTION(TableFlags, SizingStretchProp)
FLAGS_CLASS_FUNCTION(TableFlags, SizingStretchSame)
FLAGS_CLASS_FUNCTION(TableFlags, NoHostExtendX)
FLAGS_CLASS_FUNCTION(TableFlags, NoHostExtendY)
FLAGS_CLASS_FUNCTION(TableFlags, NoKeepColumnsVisible)
FLAGS_CLASS_FUNCTION(TableFlags, PreciseWidths)
FLAGS_CLASS_FUNCTION(TableFlags, NoClip)
FLAGS_CLASS_FUNCTION(TableFlags, PadOuterX)
FLAGS_CLASS_FUNCTION(TableFlags, NoPadOuterX)
FLAGS_CLASS_FUNCTION(TableFlags, NoPadInnerX)
FLAGS_CLASS_FUNCTION(TableFlags, ScrollX)
FLAGS_CLASS_FUNCTION(TableFlags, ScrollY)
FLAGS_CLASS_FUNCTION(TableFlags, SortMulti)
FLAGS_CLASS_FUNCTION(TableFlags, SortTristate)
FLAGS_CLASS_FUNCTION(TableFlags, HighlightHoveredColumn)
FLAGS_CLASS_END(TableFlags);

FLAGS_CLASS_BODY(SelectableFlags)
FLAGS_CLASS_FUNCTION(SelectableFlags, None)
FLAGS_CLASS_FUNCTION(SelectableFlags, NoAutoClosePopups)
FLAGS_CLASS_FUNCTION(SelectableFlags, SpanAllColumns)
FLAGS_CLASS_FUNCTION(SelectableFlags, AllowDoubleClick)
FLAGS_CLASS_FUNCTION(SelectableFlags, Disabled)
FLAGS_CLASS_FUNCTION(SelectableFlags, AllowOverlap)
FLAGS_CLASS_FUNCTION(SelectableFlags, Highlight)
FLAGS_CLASS_FUNCTION(SelectableFlags, SelectOnNav)
FLAGS_CLASS_END(SelectableFlags)

}
}
