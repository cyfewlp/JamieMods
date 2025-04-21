#pragma once

#include "Translation.h"
#include "common/config.h"
#include "imgui.h"

#include <array>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    template <size_t Columns>
    class TableContext
    {
        std::string                      m_name;
        ImGuiTableFlags                  m_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
        std::array<std::string, Columns> m_headers;

    public:
        TableContext(std::string &name, std::array<std::string, Columns> &headers)
            : m_name(name), m_headers(std::move(headers))
        {
        }

        auto Begin() -> bool;
        void HeadersRow();

        void CommitRow()
        {
            ImGui::TableHeadersRow();
        }

        constexpr auto GetColumns() const -> size_t
        {
            return Columns;
        }

        constexpr auto GetHeader(size_t col) const -> const std::string &
        {
            return m_headers.at(col);
        }

        constexpr auto Sortable() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_Sortable;
            return *this;
        }

        constexpr auto Hideable() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_Hideable;
            return *this;
        }

        constexpr auto Reorderable() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_Reorderable;
            return *this;
        }

        constexpr auto Resizable() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_Resizable;
            return *this;
        }

        constexpr auto NoHostExtendX() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_NoHostExtendX;
            return *this;
        }

        constexpr auto NoHostExtendY() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_NoHostExtendY;
            return *this;
        }

        constexpr auto SizingStretchProp() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_SizingStretchProp;
            return *this;
        }

        constexpr auto SizingFixedFit() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_SizingFixedFit;
            return *this;
        }

        static auto Create(const std::string_view &&nameKey, std::array<std::string_view, Columns> &&headerKey)
            -> TableContext
        {
            std::array<std::string, Columns> headers;

            auto name = Translation::TranslateNoCache(nameKey.data());
            int  idx  = 0;
            for (const auto &key : headerKey)
            {
                auto headerName = Translation::TranslateNoCache(key.data());
                headers.at(idx) = headerName;
                ++idx;
            }
            return TableContext(name, headers);
        }
    };

    struct TableBuilder
    {
        std::string_view name{};
        ImGuiTableFlags  flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;

        explicit TableBuilder(const char *name) : name(name) {}

        constexpr auto Sortable() -> TableBuilder &
        {
            flags |= ImGuiTableFlags_Sortable;
            return *this;
        }

        constexpr auto Hideable() -> TableBuilder &
        {
            flags |= ImGuiTableFlags_Hideable;
            return *this;
        }

        constexpr auto Reorderable() -> TableBuilder &
        {
            flags |= ImGuiTableFlags_Reorderable;
            return *this;
        }

        constexpr auto Resizable() -> TableBuilder &
        {
            flags |= ImGuiTableFlags_Resizable;
            return *this;
        }

        constexpr auto NoHostExtendX() -> TableBuilder &
        {
            flags |= ImGuiTableFlags_NoHostExtendX;
            return *this;
        }

        constexpr auto NoHostExtendY() -> TableBuilder &
        {
            flags |= ImGuiTableFlags_NoHostExtendY;
            return *this;
        }

        constexpr auto SizingStretchProp() -> TableBuilder &
        {
            flags |= ImGuiTableFlags_SizingStretchProp;
            return *this;
        }

        constexpr auto SizingFixedFit() -> TableBuilder &
        {
            flags |= ImGuiTableFlags_SizingFixedFit;
            return *this;
        }

        constexpr auto Begin(int columns) const -> bool
        {
            return ImGui::BeginTable(name.data(), columns, flags);
        }
    };

    struct TableHeadersBuilder
    {
        struct ColumnContext
        {
            std::string_view      name{};
            ImGuiTableColumnFlags flags = ImGuiTableColumnFlags_None;
            TableHeadersBuilder  &tableBuilder;

            explicit ColumnContext(const char *name, TableHeadersBuilder &tableBuilder)
                : name(name), tableBuilder(tableBuilder)
            {
                flags = ImGuiTableColumnFlags_None;
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

            auto Column(const char *name) -> ColumnContext
            {
                auto translated = Translation::Translate(this->name.data());
                ImGui::TableSetupColumn(translated.c_str(), flags);
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

    template <size_t Columns>
    inline auto TableContext<Columns>::Begin() -> bool
    {
        return ImGui::BeginTable(m_name.data(), Columns, m_flags);
    }

    template <size_t Columns>
    inline void TableContext<Columns>::HeadersRow()
    {
        ImGui::PushID("HeadersRow");
        for (size_t idx = 0; idx < Columns; ++idx)
        {
            ImGui::TableSetupColumn(m_headers[idx].data());
        }
        ImGui::TableHeadersRow();
        ImGui::PopID();
    }

    template <size_t Column>
    struct PagedTable : TableContext<Column>
    {
    };
}