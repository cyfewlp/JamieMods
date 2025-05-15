//
// Created by jamie on 2025/4/16.
//

#ifndef IMTHEMELOADER_H
#define IMTHEMELOADER_H

#pragma once

#include "common/config.h"
#include "common/toml++/toml.hpp"
#include "imgui.h"

#include <expected>
#include <ranges>

namespace LIBC_NAMESPACE_DECL
{
namespace ImTheme
{
static constexpr std::string_view THEME_FILE_NAME = "themes.toml";

template <typename Type>
struct converter
{
};

template <>
struct converter<ImGuiDir>
{
    static void convert(const char *value, ImGuiDir &target)
    {
        if (strcmp("Left", value) == 0)
        {
            target = ImGuiDir_Left;
        }
        else if (strcmp("Right", value) == 0)
        {
            target = ImGuiDir_Right;
        }
        else if (strcmp("Up", value) == 0)
        {
            target = ImGuiDir_Up;
        }
        else if (strcmp("Down", value) == 0)
        {
            target = ImGuiDir_Down;
        }
    }
};

template <>
struct converter<float>
{
    static void convert(const char *value, float &target)
    {
        if (const std::string strValue(value); !strValue.empty())
        {
            char       *pEnd{};
            const float result = std::strtof(value, &pEnd);
            if (*pEnd == 0)
            {
                target = result;
            }
        }
    }
};

template <>
struct converter<uint8_t>
{
    static void convert(const char *value, uint8_t &target)
    {
        if (std::string strValue(value); !strValue.empty())
        {
            char *pEnd{};
            if (const uint32_t result = std::strtoul(value, &pEnd, 10); *pEnd != 0 || result > 255)
            {
                SPDLOG_WARN("Can't get unsigned int value or value to large()");
            }
            else
            {
                target = static_cast<uint8_t>(result);
            }
        }
    }
};

template <>
struct converter<ImVec2>
{
    static void convert(const char *value, ImVec2 &target)
    {
        if (std::string strValue(value); strValue.starts_with("[") && strValue.ends_with("]"))
        {
            strValue.erase(0, 1);
            strValue.erase(strValue.end() - 1);
            std::ranges::split_view view = std::views::split(strValue, std::string_view(", "));

            std::vector<std::string> elements;
            for (const auto &part : view)
            {
                elements.emplace_back(part.begin(), part.end());
            }
            if (elements.size() == 2)
            {
                converter<float>::convert(elements[0].c_str(), target.x);
                converter<float>::convert(elements[1].c_str(), target.y);
            }
        }
    }
};

struct Theme
{
    std::string name;
    uint32_t    tomlFileNodeIndex;

    Theme(const std::string &name, const uint32_t tomlFileIndex) : name(name), tomlFileNodeIndex(tomlFileIndex) {}

    Theme(const Theme &other) = delete;

    Theme(Theme &&other) noexcept : name(std::move(other.name)), tomlFileNodeIndex(other.tomlFileNodeIndex) {}

    Theme &operator=(const Theme &other) = delete;

    Theme &operator=(Theme &&other) noexcept
    {
        if (this == &other) return *this;
        name              = std::move(other.name);
        tomlFileNodeIndex = other.tomlFileNodeIndex;
        return *this;
    }

    friend bool operator<(const Theme &lhs, const Theme &other)
    {
        std::string lower1;
        lower1.resize(lhs.name.size());
        std::string lower2{};
        lower2.resize(other.name.size());
        std::ranges::transform(lhs.name, lower1.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        std::ranges::transform(other.name, lower2.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return lower1 < lower2;
    }
};

class Loader
{
    std::vector<Theme> m_availableThemes;

public:
    struct Error final : std::runtime_error
    {
        explicit Error(const std::string &message) : std::runtime_error(message) {}
    };

    auto IsIndexInRange(int32_t index) const -> bool
    {
        return index >= 0 && index < static_cast<int32_t>(m_availableThemes.size());
    }

    void LoadThemes(std::string_view filePath);

    /**
     * @param themeIndex The Themes index.
     * @param filePath
     * @throw Error if themeIndex out of range or theme file invalid
     */
    void UseTheme(size_t themeIndex, std::string_view filePath) const noexcept(false);

    [[nodiscard]] auto GetAvailableThemes() const -> const std::vector<Theme> &
    {
        return m_availableThemes;
    }

    static auto GetInstance() -> Loader &
    {
        static Loader instance;
        return instance;
    }

private:
    static void DoUseTheme(toml::table &themeTable);

    static void ConfigImGuiStyle(const toml::node_view<toml::node> &style_node, ImGuiStyle &style);
    static void ConfigImGuiColor(const toml::node_view<toml::node> &colors_node, ImGuiStyle &style);
    static void ColorConvert(const char *colorString, ImVec4 &color);
};
};
}

#endif // IMTHEMELOADER_H
