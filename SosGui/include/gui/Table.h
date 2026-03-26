#pragma once

#include "Translation.h"
#include "common/config.h"
#include "common/imgui/ImGuiFlags.h"
#include "imgui.h"

#include <string>

namespace LIBC_NAMESPACE_DECL
{

struct TableHeadersBuilder
{
    TableHeadersBuilder() { ImGui::PushID("HeadersRow"); }

    ~TableHeadersBuilder() { ImGui::PopID(); }

    struct ColumnContext
    {
        std::string_view      name{};
        ImGuiTableColumnFlags flags             = ImGuiTableColumnFlags_None;
        float                 initWidthOrWeight = 0.0F;
        TableHeadersBuilder  &tableBuilder;

        explicit ColumnContext(const char *name, TableHeadersBuilder &tableBuilder) : name(name), tableBuilder(tableBuilder) {}

        constexpr auto WidthOrWeight(const float widthOrWeight) -> ColumnContext &
        {
            initWidthOrWeight = widthOrWeight;
            return *this;
        }

        constexpr auto Flags(ImGuiUtil::TableColumnFlags flags) -> ColumnContext &
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

        static constexpr void CommitHeadersRow() { return ImGui::TableHeadersRow(); }
    };

    auto Column(const char *columnName) -> ColumnContext { return ColumnContext(columnName, *this); }
};

} // namespace LIBC_NAMESPACE_DECL
