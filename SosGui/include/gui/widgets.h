//
// Created by jamie on 2025/4/15.
//

#ifndef WIDGETS_H
#define WIDGETS_H

#pragma once

#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
    struct MultiSelection : ImGuiSelectionBasicStorage
    {
        ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_None;

        constexpr auto NoSelectAll() -> MultiSelection &
        {
            flags |= ImGuiMultiSelectFlags_NoSelectAll;
            return *this;
        }

        constexpr auto ClearOnEscape() -> MultiSelection &
        {
            flags |= ImGuiMultiSelectFlags_ClearOnEscape;
            return *this;
        }

        constexpr auto ClearOnClickVoid() -> MultiSelection &
        {
            flags |= ImGuiMultiSelectFlags_ClearOnClickVoid;
            return *this;
        }

        constexpr auto BoxSelect1d() -> MultiSelection &
        {
            flags |= ImGuiMultiSelectFlags_BoxSelect1d;
            return *this;
        }

        auto Begin(const int itemSize) const
        {
            return ImGui::BeginMultiSelect(flags, Size, itemSize);
        }

        bool ContainsIndex(int idx)
        {
            return Contains(GetStorageIdFromIndex(idx));
        }

        static auto End()
        {
            return ImGui::EndMultiSelect();
        }
    };

}

#endif // WIDGETS_H
