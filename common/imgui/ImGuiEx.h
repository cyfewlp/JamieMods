//
// Created by jamie on 2026/1/19.
//

#pragma once

#include "common/config.h"

#include <imgui.h>
#include <type_traits>

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx
{

class FontScope
{
    bool Pushed = false;

public:
    FontScope() = default;

    explicit FontScope(ImFont *font, float fontSize = 0.f)
    {
        ImGui::PushFont(font, fontSize);
        Pushed = true;
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

    // clang-format off

    static constexpr auto Text(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_Text, .val = val};}
    static constexpr auto TextDisabled(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TextDisabled, .val = val};}
    static constexpr auto WindowBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_WindowBg, .val = val};}
    static constexpr auto ChildBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ChildBg, .val = val};}
    static constexpr auto PopupBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_PopupBg, .val = val};}
    static constexpr auto Border(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_Border, .val = val};}
    static constexpr auto BorderShadow(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_BorderShadow, .val = val};}
    static constexpr auto FrameBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_FrameBg, .val = val};}
    static constexpr auto FrameBgHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_FrameBgHovered, .val = val};}
    static constexpr auto FrameBgActive(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_FrameBgActive, .val = val};}
    static constexpr auto TitleBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TitleBg, .val = val};}
    static constexpr auto TitleBgActive(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TitleBgActive, .val = val};}
    static constexpr auto TitleBgCollapsed(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TitleBgCollapsed, .val = val};}
    static constexpr auto MenuBarBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_MenuBarBg, .val = val};}
    static constexpr auto ScrollbarBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ScrollbarBg, .val = val};}
    static constexpr auto ScrollbarGrab(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ScrollbarGrab, .val = val};}
    static constexpr auto ScrollbarGrabHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ScrollbarGrabHovered, .val = val};}
    static constexpr auto ScrollbarGrabActive(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ScrollbarGrabActive, .val = val};}
    static constexpr auto CheckMark(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_CheckMark, .val = val};}
    static constexpr auto SliderGrab(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_SliderGrab, .val = val};}
    static constexpr auto SliderGrabActive(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_SliderGrabActive, .val = val};}
    static constexpr auto Button(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_Button, .val = val};}
    static constexpr auto ButtonHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ButtonHovered, .val = val};}
    static constexpr auto ButtonActive(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ButtonActive, .val = val};}
    static constexpr auto Header(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_Header, .val = val};}
    static constexpr auto HeaderHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_HeaderHovered, .val = val};}
    static constexpr auto HeaderActive(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_HeaderActive, .val = val};}
    static constexpr auto Separator(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_Separator, .val = val};}
    static constexpr auto SeparatorHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_SeparatorHovered, .val = val};}
    static constexpr auto SeparatorActive(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_SeparatorActive, .val = val};}
    static constexpr auto ResizeGrip(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ResizeGrip, .val = val};}
    static constexpr auto ResizeGripHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ResizeGripHovered, .val = val};}
    static constexpr auto ResizeGripActive(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ResizeGripActive, .val = val};}
    static constexpr auto InputTextCursor(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_InputTextCursor, .val = val};}
    static constexpr auto TabHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TabHovered, .val = val};}
    static constexpr auto Tab(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_Tab, .val = val};}
    static constexpr auto TabSelected(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TabSelected, .val = val};}
    static constexpr auto TabSelectedOverline(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TabSelectedOverline, .val = val};}
    static constexpr auto TabDimmed(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TabDimmed, .val = val};}
    static constexpr auto TabDimmedSelected(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TabDimmedSelected, .val = val};}
    static constexpr auto TabDimmedSelectedOverline(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TabDimmedSelectedOverline, .val = val};}
    static constexpr auto PlotLines(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_PlotLines, .val = val};}
    static constexpr auto PlotLinesHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_PlotLinesHovered, .val = val};}
    static constexpr auto PlotHistogram(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_PlotHistogram, .val = val};}
    static constexpr auto PlotHistogramHovered(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_PlotHistogramHovered, .val = val};}
    static constexpr auto TableHeaderBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TableHeaderBg, .val = val};}
    static constexpr auto TableBorderStrong(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TableBorderStrong, .val = val};}
    static constexpr auto TableBorderLight(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TableBorderLight, .val = val};}
    static constexpr auto TableRowBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TableRowBg, .val = val};}
    static constexpr auto TableRowBgAlt(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TableRowBgAlt, .val = val};}
    static constexpr auto TextLink(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TextLink, .val = val};}
    static constexpr auto TextSelectedBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TextSelectedBg, .val = val};}
    static constexpr auto TreeLines(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_TreeLines, .val = val};}
    static constexpr auto DragDropTarget(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_DragDropTarget, .val = val};}
    static constexpr auto DragDropTargetBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_DragDropTargetBg, .val = val};}
    static constexpr auto UnsavedMarker(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_UnsavedMarker, .val = val};}
    static constexpr auto NavCursor(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_NavCursor, .val = val};}
    static constexpr auto NavWindowingHighlight(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_NavWindowingHighlight, .val = val};}
    static constexpr auto NavWindowingDimBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_NavWindowingDimBg, .val = val};}
    static constexpr auto ModalWindowDimBg(const ImVec4&val) -> ColorHolder {return {.idx = ImGuiCol_ModalWindowDimBg, .val = val};}

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

    // clang-format off
    static constexpr auto Alpha(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_Alpha, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto DisabledAlpha(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_DisabledAlpha, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto WindowPadding(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_WindowPadding, .type = Type::Vec2, .val = val};}
    static constexpr auto WindowRounding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_WindowRounding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto WindowBorderSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_WindowBorderSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto WindowMinSize(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_WindowMinSize, .type = Type::Vec2, .val = val};}
    static constexpr auto WindowTitleAlign(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_WindowTitleAlign, .type = Type::Vec2, .val = val};}
    static constexpr auto ChildRounding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_ChildRounding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto ChildBorderSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_ChildBorderSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto PopupRounding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_PopupRounding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto PopupBorderSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_PopupBorderSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto FramePadding(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_FramePadding, .type = Type::Vec2, .val = val};}
    static constexpr auto FrameRounding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_FrameRounding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto FrameBorderSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_FrameBorderSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto ItemSpacing(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_ItemSpacing, .type = Type::Vec2, .val = val};}
    static constexpr auto ItemInnerSpacing(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_ItemInnerSpacing, .type = Type::Vec2, .val = val};}
    static constexpr auto IndentSpacing(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_IndentSpacing, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto CellPadding(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_CellPadding, .type = Type::Vec2, .val = val};}
    static constexpr auto ScrollbarSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_ScrollbarSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto ScrollbarRounding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_ScrollbarRounding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto ScrollbarPadding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_ScrollbarPadding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto GrabMinSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_GrabMinSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto GrabRounding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_GrabRounding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto ImageBorderSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_ImageBorderSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TabRounding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TabRounding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TabBorderSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TabBorderSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TabMinWidthBase(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TabMinWidthBase, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TabMinWidthShrink(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TabMinWidthShrink, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TabBarBorderSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TabBarBorderSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TabBarOverlineSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TabBarOverlineSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TableAngledHeadersAngle(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TableAngledHeadersAngle, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TableAngledHeadersTextAlign(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_TableAngledHeadersTextAlign, .type = Type::Vec2, .val = val};}
    static constexpr auto TreeLinesSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TreeLinesSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto TreeLinesRounding(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_TreeLinesRounding, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto ButtonTextAlign(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_ButtonTextAlign, .type = Type::Vec2, .val = val};}
    static constexpr auto SelectableTextAlign(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_SelectableTextAlign, .type = Type::Vec2, .val = val};}
    static constexpr auto SeparatorTextBorderSize(float val) -> StyleHolder{return {.idx = ImGuiStyleVar_SeparatorTextBorderSize, .type = Type::Float, .val = ImVec2(val, 0)};}
    static constexpr auto SeparatorTextAlign(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_SeparatorTextAlign, .type = Type::Vec2, .val = val};}
    static constexpr auto SeparatorTextPadding(const ImVec2 &val) -> StyleHolder{return {.idx = ImGuiStyleVar_SeparatorTextPadding, .type = Type::Vec2, .val = val};}

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

    auto Push(StyleHolder &&s)  -> StyleGuard &
    {
        if (s.type == StyleHolder::Type::Float)
        {
            ImGui::PushStyleVar(s.idx, s.val.x);
        }
        else
        {
            ImGui::PushStyleVar(s.idx, s.val);
        }
        varCount++;
        return *this;
    }

    auto Push(ColorHolder &&s)  -> StyleGuard &
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
