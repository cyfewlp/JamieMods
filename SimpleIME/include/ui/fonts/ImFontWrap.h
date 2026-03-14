//
// Created by jamie on 2026/1/16.
//

#pragma once

#include <string>

struct ImFont;

namespace Ime
{
class ImFontWrap
{
    ImFont                  *font  = nullptr;
    bool                     owner = false;
    std::vector<std::string> m_fontNames;
    std::vector<std::string> m_fontPathList;

public:
    explicit ImFontWrap(ImFont *imFont = nullptr, bool a_owner = false) : font(imFont), owner(a_owner && imFont != nullptr) {}

    explicit ImFontWrap(ImFont *imFont, std::string_view fontName, std::string_view fontPath, bool a_owner = false);

    ImFontWrap(ImFontWrap &other) noexcept = delete;
    auto operator=(const ImFontWrap &other) -> ImFontWrap &;

    ImFontWrap(ImFontWrap &&other) noexcept
        : font(other.font), owner(other.owner), m_fontNames(std::move(other.m_fontNames)), m_fontPathList(std::move(other.m_fontPathList))
    {
        other.font  = nullptr;
        other.owner = false;
        other.m_fontNames.clear();
        other.m_fontPathList.clear();
    }

    auto operator=(ImFontWrap &&other) noexcept -> ImFontWrap &;

    ~ImFontWrap() { Cleanup(); }

    void AddFontInfo(std::string_view fontName, std::string_view fontPath)
    {
        m_fontNames.emplace_back(fontName);
        m_fontPathList.emplace_back(fontPath);
    }

    [[nodiscard]] auto IsOwner() const -> bool { return owner; }

    [[nodiscard]] auto IsCommittable() const -> bool;
    [[nodiscard]] auto IsCommittableSingleFont() const -> bool;

    [[nodiscard]] auto GetFontNames() const -> const std::vector<std::string> & { return m_fontNames; }

    [[nodiscard]] auto GetFontPathList() const -> const std::vector<std::string> & { return m_fontPathList; }

    auto GetFontPathList() -> std::vector<std::string> & { return m_fontPathList; }

    [[nodiscard]] auto GetFontNameOr(size_t index, std::string_view value = "") const -> std::string_view
    {
        if (m_fontNames.size() <= index) return value;
        return m_fontNames[index];
    }

    [[nodiscard]] auto GetFontPathOr(size_t index, std::string_view value = "") const -> std::string_view
    {
        if (m_fontPathList.size() <= index) return value;
        return m_fontPathList[index];
    }

    friend auto operator==(const ImFontWrap &lhs, const ImFontWrap &rhs) -> bool { return lhs.font == rhs.font; }

    [[nodiscard]] auto GetFont() const -> const ImFont * { return font; }

    [[nodiscard]] auto UnsafeGetFont() const -> ImFont * { return font; }

    auto TakeFont() -> ImFont *
    {
        auto *ptr = font;
        font      = nullptr;
        owner     = false;
        return ptr;
    }

    auto operator->() const -> ImFont * { return font; }

    void Cleanup();

private:
    inline void RemoveFontIfOwned() const;
};
} // namespace Ime
