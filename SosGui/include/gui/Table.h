#pragma once

#include "Translation.h"
#include "common/config.h"
#include "imgui.h"

#include <array>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
struct TableFlags
{
    ImGuiTableFlags flags = ImGuiTableFlags_None;

    constexpr auto RowBg() -> TableFlags &
    {
        flags |= ImGuiTableFlags_RowBg;
        return *this;
    }

    constexpr auto BordersInnerH() -> TableFlags &
    {
        flags |= ImGuiTableFlags_BordersInnerH;
        return *this;
    }

    constexpr auto Borders() -> TableFlags &
    {
        flags |= ImGuiTableFlags_Borders;
        return *this;
    }

    constexpr auto Sortable() -> TableFlags &
    {
        flags |= ImGuiTableFlags_Sortable;
        return *this;
    }

    constexpr auto Hideable() -> TableFlags &
    {
        flags |= ImGuiTableFlags_Hideable;
        return *this;
    }

    constexpr auto Reorderable() -> TableFlags &
    {
        flags |= ImGuiTableFlags_Reorderable;
        return *this;
    }

    constexpr auto Resizable() -> TableFlags &
    {
        flags |= ImGuiTableFlags_Resizable;
        return *this;
    }

    constexpr auto NoHostExtendX() -> TableFlags &
    {
        flags |= ImGuiTableFlags_NoHostExtendX;
        return *this;
    }

    constexpr auto NoHostExtendY() -> TableFlags &
    {
        flags |= ImGuiTableFlags_NoHostExtendY;
        return *this;
    }

    constexpr auto SizingStretchProp() -> TableFlags &
    {
        flags |= ImGuiTableFlags_SizingStretchProp;
        return *this;
    }

    constexpr auto SizingFixedFit() -> TableFlags &
    {
        flags |= ImGuiTableFlags_SizingFixedFit;
        return *this;
    }

    constexpr auto ScrollX() -> TableFlags &
    {
        flags |= ImGuiTableFlags_ScrollX;
        return *this;
    }

    constexpr auto ScrollY() -> TableFlags &
    {
        flags |= ImGuiTableFlags_ScrollY;
        return *this;
    }
};

struct TableHeadersBuilder
{
    TableHeadersBuilder()
    {
        ImGui::PushID("HeadersRow");
    }

    ~TableHeadersBuilder()
    {
        ImGui::PopID();
    }

    struct ColumnContext
    {
        std::string_view      name{};
        ImGuiTableColumnFlags flags             = ImGuiTableColumnFlags_None;
        float                 initWidthOrWeight = 0.0F;
        TableHeadersBuilder  &tableBuilder;

        explicit ColumnContext(const char *name, TableHeadersBuilder &tableBuilder)
            : name(name), tableBuilder(tableBuilder)
        {
            flags = ImGuiTableColumnFlags_None;
        }

        constexpr auto WidthOrWeight(const float widthOrWeight) -> ColumnContext &
        {
            initWidthOrWeight = widthOrWeight;
            return *this;
        }

        constexpr auto DefaultSort() -> ColumnContext &
        {
            flags |= ImGuiTableColumnFlags_DefaultSort;
            return *this;
        }

        constexpr auto NoHide() -> ColumnContext &
        {
            flags |= ImGuiTableColumnFlags_NoHide;
            return *this;
        }

        constexpr auto NoSort() -> ColumnContext &
        {
            flags |= ImGuiTableColumnFlags_NoSort;
            return *this;
        }

        constexpr auto WidthFixed() -> ColumnContext &
        {
            flags |= ImGuiTableColumnFlags_WidthFixed;
            return *this;
        }

        constexpr auto WidthStretch() -> ColumnContext &
        {
            flags |= ImGuiTableColumnFlags_WidthStretch;
            return *this;
        }

        constexpr auto Setup() -> ColumnContext &
        {
            auto translated = Translation::Translate(this->name.data());
            ImGui::TableSetupColumn(translated.c_str(), flags, initWidthOrWeight);
            return *this;
        }

        auto Column(const char *name) -> ColumnContext
        {
            Setup();
            flags = ImGuiTableColumnFlags_None;
            return tableBuilder.Column(name);
        }

        constexpr void CommitHeadersRow()
        {
            return ImGui::TableHeadersRow();
        }
    };

    auto Column(const char *columnName) -> ColumnContext
    {
        return ColumnContext(columnName, *this);
    }
};

}