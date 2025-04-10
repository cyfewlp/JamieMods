#pragma once

#include "Translation.h"
#include "common/config.h"
#include "imgui.h"

#include <array>
#include <string_view>

namespace LIBC_NAMESPACE_DECL
{
    template <size_t Columns>
    class TableContext
    {
        std::string                      m_name;
        ImGuiTableFlags                  m_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
        std::array<std::string, Columns> m_headers;

    public:
        struct ColumnContext
        {
            std::string            colName;
            ImGuiTableColumnFlags  flags;
            TableContext<Columns> &context;

            ColumnContext(const std::string &colName, TableContext<Columns> &context)
                : colName(colName), context(context)
            {
                flags = ImGuiTableColumnFlags_None;
            }

            constexpr auto DefaultSort() -> ColumnContext &
            {
                flags |= ImGuiTableColumnFlags_DefaultSort;
                return *this;
            }

            constexpr auto NoSort() -> ColumnContext &
            {
                flags |= ImGuiTableColumnFlags_NoSort;
                return *this;
            }

            constexpr auto Setup() -> TableContext &
            {
                ImGui::TableSetupColumn(colName.data(), flags);
                return context;
            }
        };

        TableContext(std::string &name, std::array<std::string, Columns> &headers)
            : m_name(name), m_headers(std::move(headers))
        {
        }

        auto Begin() -> bool;
        void HeadersRow();

        constexpr auto Column(size_t colIdx) -> ColumnContext
        {
            const auto &name = m_headers.at(colIdx);
            return ColumnContext(name, *this);
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

        constexpr auto SizingStretchProp() -> TableContext &
        {
            m_flags |= ImGuiTableFlags_SizingStretchProp;
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
}