#pragma once

#include "Translation.h"
#include "common/config.h"
#include "imgui.h"

#include <string>

namespace LIBC_NAMESPACE_DECL
{
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
        }

        constexpr auto WidthOrWeight(const float widthOrWeight) -> ColumnContext &
        {
            initWidthOrWeight = widthOrWeight;
            return *this;
        }

        constexpr auto Flags(TableColumnFlags flags) -> ColumnContext &
        {
            this->flags = flags;
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

        static constexpr void CommitHeadersRow()
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