//
// Created by jamie on 2026/1/19.
//

#pragma once

#include "../path_utils.h"

#include <imgui.h>
#include <utility>

namespace ImGuiEx
{

constexpr float ALIGN_LEFT   = 0.0F;
constexpr float ALIGN_RIGHT  = 1.0F;
constexpr float ALIGN_CENTER = 0.5F;

inline auto TextStart(const std::string_view text) -> const char *
{
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return text.data();
}

inline auto TextEnd(const std::string_view text) -> const char *
{
    return text.data() + text.size(); // NOLINT(*-pro-bounds-pointer-arithmetic)
}

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
    int m_varCount   = 0;
    int m_colorCount = 0;

public:
    StyleGuard()                              = default;
    StyleGuard(const StyleGuard &)            = delete;
    StyleGuard &operator=(const StyleGuard &) = delete;

    StyleGuard(StyleGuard &&other) noexcept
        : m_varCount(std::exchange(other.m_varCount, 0)), m_colorCount(std::exchange(other.m_colorCount, 0))
    {
    }

    StyleGuard &operator=(StyleGuard &&other) noexcept
    {
        if (this != &other)
        {
            Pop();
            m_varCount   = std::exchange(other.m_varCount, 0);
            m_colorCount = std::exchange(other.m_colorCount, 0);
        }
        return *this;
    }

    template <ImGuiStyleVar_ StyleVar, typename T>
    auto Style(const T &val) && -> StyleGuard &&
    {
        ImGui::PushStyleVar(StyleVar, val);
        m_varCount++;
        return std::move(*this);
    }

    template <ImGuiStyleVar_ StyleVar, typename T>
    auto Style(const T &val) & -> StyleGuard &
    {
        ImGui::PushStyleVar(StyleVar, val);
        m_varCount++;
        return *this;
    }

    template <ImGuiStyleVar_ StyleVar>
    auto Style(const ImVec2 &val) && -> StyleGuard &&
    {
        ImGui::PushStyleVar(StyleVar, val);
        m_varCount++;
        return std::move(*this);
    }

    template <ImGuiStyleVar_ StyleVar>
    auto Style(const ImVec2 &val) & -> StyleGuard &
    {
        ImGui::PushStyleVar(StyleVar, val);
        m_varCount++;
        return *this;
    }

    template <ImGuiCol_ ColorIdx>
    auto Color(const ImVec4 &col) && -> StyleGuard &&
    {
        ImGui::PushStyleColor(ColorIdx, col);
        m_colorCount++;
        return std::move(*this);
    }

    template <ImGuiCol_ ColorIdx>
    auto Color(const ImVec4 &col) & -> StyleGuard &
    {
        ImGui::PushStyleColor(ColorIdx, col);
        m_colorCount++;
        return *this;
    }

    void Pop() noexcept
    {
        if (m_varCount > 0)
        {
            ImGui::PopStyleVar(m_varCount);
            m_varCount = 0;
        }
        if (m_colorCount > 0)
        {
            ImGui::PopStyleColor(m_colorCount);
            m_colorCount = 0;
        }
    }

    ~StyleGuard()
    {
        Pop();
    }
};
} // namespace ImGuiEx
