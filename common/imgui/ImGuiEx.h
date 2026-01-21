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
class StyleScope
{
    int varCount   = 0;
    int colorCount = 0;

public:
    StyleScope() = default;

    StyleScope(const StyleScope &)            = delete;
    StyleScope &operator=(const StyleScope &) = delete;
    StyleScope &operator=(StyleScope &&other) = delete;

    StyleScope(StyleScope &&other) noexcept : varCount(other.varCount), colorCount(other.colorCount)
    {
        other.varCount   = 0;
        other.colorCount = 0;
    }

    ~StyleScope()
    {
        if (varCount > 0)
        {
            ImGui::PopStyleVar(varCount);
        }
        if (colorCount > 0)
        {
            ImGui::PopStyleColor(colorCount);
        }
    }

    // 适配各种 PushStyleVar 重载
    template <typename T>
    auto PushVar(ImGuiStyleVar idx, T val) && -> StyleScope &&
    {
        ImGui::PushStyleVar(idx, val);
        varCount++;
        return std::move(*this);
    }

    auto PushVarX(ImGuiStyleVar idx, float val_x) -> StyleScope &&
    {
        ImGui::PushStyleVarX(idx, val_x);
        varCount++;
        return std::move(*this);
    }

    auto PushVarY(ImGuiStyleVar idx, float val_y) -> StyleScope &&
    {
        ImGui::PushStyleVarY(idx, val_y);
        varCount++;
        return std::move(*this);
    }

    auto PushColor(ImGuiCol idx, ImU32 col) && -> StyleScope &&
    {
        ImGui::PushStyleColor(idx, col);
        colorCount++;
        return std::move(*this);
    }

    auto PushColor(ImGuiCol idx, const ImVec4 &col) && -> StyleScope &&
    {
        ImGui::PushStyleColor(idx, col);
        colorCount++;
        return std::move(*this);
    }

    template <typename F>
    decltype(auto) Draw(F &&action) &&
    {
        return std::forward<F>(action)();
    }
};

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
