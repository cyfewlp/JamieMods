//
// Created by jamie on 2025/5/26.
//

#ifndef IMTHEMESLOADER_H
#define IMTHEMESLOADER_H

#include "common/config.h"

#include <algorithm>
#include <expected>
#include <memory>
#include <string>
#include <vector>

struct ImGuiStyle;

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiUtil
{
struct Theme
{
    size_t      tomlArrayIndex;
    std::string name;

    Theme(const size_t index, const std::string &name) : tomlArrayIndex(index), name(name) {}

    Theme(const Theme &other) = delete;

    Theme(Theme &&other) noexcept : tomlArrayIndex(other.tomlArrayIndex), name(std::move(other.name)) {}

    Theme &operator=(const Theme &other) = delete;

    Theme &operator=(Theme &&other) noexcept
    {
        if (this == &other) return *this;
        name           = std::move(other.name);
        tomlArrayIndex = other.tomlArrayIndex;
        return *this;
    }

    friend bool operator<(const Theme &lhs, const Theme &other)
    {
        std::string lower1(lhs.name.size(), '\0');
        std::string lower2(other.name.size(), '\0');
        std::ranges::transform(lhs.name, lower1.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        std::ranges::transform(other.name, lower2.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return lower1 < lower2;
    }
};

class ThemesLoader
{
    std::vector<Theme>      m_availableThemes;
    std::unique_ptr<char[]> m_filePath;

    enum DefaultThemeIndex : std::size_t
    {
        DefaultThemeIndex_DARK    = static_cast<size_t>(-1),
        DefaultThemeIndex_LIGHT   = static_cast<size_t>(-2),
        DefaultThemeIndex_CLASSIC = static_cast<size_t>(-3),
        DefaultThemeIndex_Count   = 3,
    };

public:
    static constexpr std::string_view DEFAULT_THEME_FILE = "themes.toml";

    explicit ThemesLoader(const std::string_view &path)
    {
        m_filePath = std::make_unique<char[]>(path.size() + 1);
        std::ranges::copy(path, m_filePath.get());
    }

    struct Error final : std::runtime_error
    {
        explicit Error(const std::string &what) : std::runtime_error(what) {}
    };

    void LoadThemes();
    /**
     * @param themeIndex The Themes index.
     */
    auto UseTheme(size_t themeIndex) const noexcept -> std::expected<void, std::string>;

    [[nodiscard]] auto GetThemes() const -> const std::vector<Theme> &
    {
        return m_availableThemes;
    }
};
}
}

#endif // IMTHEMESLOADER_H
