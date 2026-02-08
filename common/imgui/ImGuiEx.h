//
// Created by jamie on 2026/1/19.
//

#pragma once

#include "common/utils.h"

#include <imgui.h>
#include <utility>

namespace ImGuiEx
{

constexpr float ALIGN_LEFT   = 0.0F;
constexpr float ALIGN_RIGHT  = 1.0F;
constexpr float ALIGN_CENTER = 0.5F;

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

    template <typename T>
    auto PushVar(ImGuiStyleVar idx, T val) -> StyleGuard &
    {
        ImGui::PushStyleVar(idx, val);
        varCount++;
        return *this;
    }

    auto PushColor(ImGuiCol idx, const ImVec4 &col) -> StyleGuard &
    {
        ImGui::PushStyleColor(idx, col);
        colorCount++;
        return *this;
    }

    constexpr auto Style_Alpha(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_Alpha, val);
        return *this;
    }

    constexpr auto Style_DisabledAlpha(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_DisabledAlpha, val);
        return *this;
    }

    constexpr auto Style_WindowPadding(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_WindowPadding, val);
        return *this;
    }

    constexpr auto Style_WindowRounding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_WindowRounding, val);
        return *this;
    }

    constexpr auto Style_WindowBorderSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_WindowBorderSize, val);
        return *this;
    }

    constexpr auto Style_WindowMinSize(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_WindowMinSize, val);
        return *this;
    }

    constexpr auto Style_WindowTitleAlign(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_WindowTitleAlign, val);
        return *this;
    }

    constexpr auto Style_ChildRounding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ChildRounding, val);
        return *this;
    }

    constexpr auto Style_ChildBorderSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ChildBorderSize, val);
        return *this;
    }

    constexpr auto Style_PopupRounding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_PopupRounding, val);
        return *this;
    }

    constexpr auto Style_PopupBorderSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_PopupBorderSize, val);
        return *this;
    }

    constexpr auto Style_FramePadding(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_FramePadding, val);
        return *this;
    }

    constexpr auto Style_FrameRounding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_FrameRounding, val);
        return *this;
    }

    constexpr auto Style_FrameBorderSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_FrameBorderSize, val);
        return *this;
    }

    constexpr auto Style_ItemSpacing(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ItemSpacing, val);
        return *this;
    }

    constexpr auto Style_ItemInnerSpacing(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ItemInnerSpacing, val);
        return *this;
    }

    constexpr auto Style_IndentSpacing(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_IndentSpacing, val);
        return *this;
    }

    constexpr auto Style_CellPadding(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_CellPadding, val);
        return *this;
    }

    constexpr auto Style_ScrollbarSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ScrollbarSize, val);
        return *this;
    }

    constexpr auto Style_ScrollbarRounding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ScrollbarRounding, val);
        return *this;
    }

    constexpr auto Style_ScrollbarPadding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ScrollbarPadding, val);
        return *this;
    }

    constexpr auto Style_GrabMinSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_GrabMinSize, val);
        return *this;
    }

    constexpr auto Style_GrabRounding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_GrabRounding, val);
        return *this;
    }

    constexpr auto Style_ImageBorderSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ImageBorderSize, val);
        return *this;
    }

    constexpr auto Style_TabRounding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TabRounding, val);
        return *this;
    }

    constexpr auto Style_TabBorderSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TabBorderSize, val);
        return *this;
    }

    constexpr auto Style_TabMinWidthBase(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TabMinWidthBase, val);
        return *this;
    }

    constexpr auto Style_TabMinWidthShrink(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TabMinWidthShrink, val);
        return *this;
    }

    constexpr auto Style_TabBarBorderSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TabBarBorderSize, val);
        return *this;
    }

    constexpr auto Style_TabBarOverlineSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TabBarOverlineSize, val);
        return *this;
    }

    constexpr auto Style_TableAngledHeadersAngle(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TableAngledHeadersAngle, val);
        return *this;
    }

    constexpr auto Style_TableAngledHeadersTextAlign(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TableAngledHeadersTextAlign, val);
        return *this;
    }

    constexpr auto Style_TreeLinesSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TreeLinesSize, val);
        return *this;
    }

    constexpr auto Style_TreeLinesRounding(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_TreeLinesRounding, val);
        return *this;
    }

    constexpr auto Style_ButtonTextAlign(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_ButtonTextAlign, val);
        return *this;
    }

    constexpr auto Style_SelectableTextAlign(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_SelectableTextAlign, val);
        return *this;
    }

    constexpr auto Style_SeparatorTextBorderSize(float val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_SeparatorTextBorderSize, val);
        return *this;
    }

    constexpr auto Style_SeparatorTextAlign(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_SeparatorTextAlign, val);
        return *this;
    }

    constexpr auto Style_SeparatorTextPadding(const ImVec2 &val) -> StyleGuard &
    {
        PushVar(ImGuiStyleVar_SeparatorTextPadding, val);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    constexpr auto Color_Text(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_Text, val);
        return *this;
    }

    constexpr auto Color_TextDisabled(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TextDisabled, val);
        return *this;
    }

    constexpr auto Color_WindowBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_WindowBg, val);
        return *this;
    }

    constexpr auto Color_ChildBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ChildBg, val);
        return *this;
    }

    constexpr auto Color_PopupBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_PopupBg, val);
        return *this;
    }

    constexpr auto Color_Border(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_Border, val);
        return *this;
    }

    constexpr auto Color_BorderShadow(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_BorderShadow, val);
        return *this;
    }

    constexpr auto Color_FrameBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_FrameBg, val);
        return *this;
    }

    constexpr auto Color_FrameBgHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_FrameBgHovered, val);
        return *this;
    }

    constexpr auto Color_FrameBgActive(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_FrameBgActive, val);
        return *this;
    }

    constexpr auto Color_TitleBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TitleBg, val);
        return *this;
    }

    constexpr auto Color_TitleBgActive(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TitleBgActive, val);
        return *this;
    }

    constexpr auto Color_TitleBgCollapsed(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TitleBgCollapsed, val);
        return *this;
    }

    constexpr auto Color_MenuBarBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_MenuBarBg, val);
        return *this;
    }

    constexpr auto Color_ScrollbarBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ScrollbarBg, val);
        return *this;
    }

    constexpr auto Color_ScrollbarGrab(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ScrollbarGrab, val);
        return *this;
    }

    constexpr auto Color_ScrollbarGrabHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ScrollbarGrabHovered, val);
        return *this;
    }

    constexpr auto Color_ScrollbarGrabActive(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ScrollbarGrabActive, val);
        return *this;
    }

    constexpr auto Color_CheckMark(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_CheckMark, val);
        return *this;
    }

    constexpr auto Color_SliderGrab(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_SliderGrab, val);
        return *this;
    }

    constexpr auto Color_SliderGrabActive(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_SliderGrabActive, val);
        return *this;
    }

    constexpr auto Color_Button(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_Button, val);
        return *this;
    }

    constexpr auto Color_ButtonHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ButtonHovered, val);
        return *this;
    }

    constexpr auto Color_ButtonActive(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ButtonActive, val);
        return *this;
    }

    constexpr auto Color_Header(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_Header, val);
        return *this;
    }

    constexpr auto Color_HeaderHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_HeaderHovered, val);
        return *this;
    }

    constexpr auto Color_HeaderActive(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_HeaderActive, val);
        return *this;
    }

    constexpr auto Color_Separator(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_Separator, val);
        return *this;
    }

    constexpr auto Color_SeparatorHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_SeparatorHovered, val);
        return *this;
    }

    constexpr auto Color_SeparatorActive(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_SeparatorActive, val);
        return *this;
    }

    constexpr auto Color_ResizeGrip(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ResizeGrip, val);
        return *this;
    }

    constexpr auto Color_ResizeGripHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ResizeGripHovered, val);
        return *this;
    }

    constexpr auto Color_ResizeGripActive(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ResizeGripActive, val);
        return *this;
    }

    constexpr auto Color_InputTextCursor(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_InputTextCursor, val);
        return *this;
    }

    constexpr auto Color_TabHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TabHovered, val);
        return *this;
    }

    constexpr auto Color_Tab(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_Tab, val);
        return *this;
    }

    constexpr auto Color_TabSelected(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TabSelected, val);
        return *this;
    }

    constexpr auto Color_TabSelectedOverline(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TabSelectedOverline, val);
        return *this;
    }

    constexpr auto Color_TabDimmed(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TabDimmed, val);
        return *this;
    }

    constexpr auto Color_TabDimmedSelected(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TabDimmedSelected, val);
        return *this;
    }

    constexpr auto Color_TabDimmedSelectedOverline(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TabDimmedSelectedOverline, val);
        return *this;
    }

    constexpr auto Color_PlotLines(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_PlotLines, val);
        return *this;
    }

    constexpr auto Color_PlotLinesHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_PlotLinesHovered, val);
        return *this;
    }

    constexpr auto Color_PlotHistogram(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_PlotHistogram, val);
        return *this;
    }

    constexpr auto Color_PlotHistogramHovered(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_PlotHistogramHovered, val);
        return *this;
    }

    constexpr auto Color_TableHeaderBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TableHeaderBg, val);
        return *this;
    }

    constexpr auto Color_TableBorderStrong(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TableBorderStrong, val);
        return *this;
    }

    constexpr auto Color_TableBorderLight(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TableBorderLight, val);
        return *this;
    }

    constexpr auto Color_TableRowBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TableRowBg, val);
        return *this;
    }

    constexpr auto Color_TableRowBgAlt(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TableRowBgAlt, val);
        return *this;
    }

    constexpr auto Color_TextLink(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TextLink, val);
        return *this;
    }

    constexpr auto Color_TextSelectedBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TextSelectedBg, val);
        return *this;
    }

    constexpr auto Color_TreeLines(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_TreeLines, val);
        return *this;
    }

    constexpr auto Color_DragDropTarget(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_DragDropTarget, val);
        return *this;
    }

    constexpr auto Color_DragDropTargetBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_DragDropTargetBg, val);
        return *this;
    }

    constexpr auto Color_UnsavedMarker(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_UnsavedMarker, val);
        return *this;
    }

    constexpr auto Color_NavCursor(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_NavCursor, val);
        return *this;
    }

    constexpr auto Color_NavWindowingHighlight(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_NavWindowingHighlight, val);
        return *this;
    }

    constexpr auto Color_NavWindowingDimBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_NavWindowingDimBg, val);
        return *this;
    }

    constexpr auto Color_ModalWindowDimBg(const ImVec4 &val) -> StyleGuard &
    {
        PushColor(ImGuiCol_ModalWindowDimBg, val);
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

constexpr auto HexToU32(std::string_view hex) -> std::optional<ImU32>
{
    if (hex.size() != 7 || !hex.starts_with("#")) return std::nullopt;

    auto hexSv1 = hex.substr(1);

    auto get_byte = [&](size_t idx, uint32_t &col) -> bool {
        auto p1 = utils::HexCharToInt(hexSv1[idx * 2]);
        if (p1 == UINT8_MAX) return false;
        auto p2 = utils::HexCharToInt(hexSv1[idx * 2 + 1]);
        if (p2 == UINT8_MAX) return false;
        col = p1 << 4 | p2;
        return true;
    };

    uint32_t r = 0, g = 0, b = 0;
    if (get_byte(0, r) && get_byte(1, g) && get_byte(2, b))
    {
        return IM_COL32_A_MASK | b << IM_COL32_B_SHIFT | g << IM_COL32_G_SHIFT | r;
    }
    return std::nullopt;
}

} // namespace ImGuiEx
