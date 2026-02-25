#ifndef IME_TEXTEDITOR_H
#define IME_TEXTEDITOR_H

#pragma once

#include <TextStor.h>
#include <shared_mutex>
#include <string>

namespace Ime
{
/**
 * @class TextEditor
 * @brief A simple text editor to manage the composition text.
 *
 * The `acpStart` and `acpEnd` in `TS_SELECTION_ACP` defined with LONG type.
 * As agreed, the negative value of `acpStart` and `acpEnd` will be treated as the end of the text.
 */
class TextEditor
{
public:
    TextEditor()                                                = default;
    ~TextEditor()                                               = default;
    TextEditor(const TextEditor &other)                         = delete;
    TextEditor(TextEditor &&other) noexcept                     = delete;
    auto operator=(const TextEditor &other) -> TextEditor &     = delete;
    auto operator=(TextEditor &&other) noexcept -> TextEditor & = delete;

    auto Select(int32_t acpStart, int32_t acpEnd) -> void;
    auto Select(const TS_SELECTION_ACP *pSelectionAcp) -> void;

    auto SelectAll()
    {
        const std::unique_lock lock(m_mutex);
        m_acpSelection.acpStart = 0;
        m_acpSelection.acpEnd   = -1;
    }

    void GetSelection(LONG *pAcpStart, LONG *pAcpEnd) const;
    auto GetSelection(TS_SELECTION_ACP *pSelectionAcp) const -> void;
    auto InsertText(const wchar_t *pwszText, size_t cch) -> bool;

    auto InsertText(std::wstring_view wsTextView) -> bool { return InsertText(wsTextView.data(), wsTextView.size()); }

    void ClearText();

    /**
     * @return The editor text size in characters
     */
    constexpr auto GetTextSize(size_t &charSize) const -> void
    {
        const std::shared_lock lock(m_mutex);
        charSize = m_editorText.size();
    }

    /**
     * @return The editor text size in characters
     */
    [[nodiscard]] constexpr auto GetTextSize() const -> size_t
    {
        const std::shared_lock lock(m_mutex);
        return m_editorText.size();
    }

    /**
     * Require copied from the specified offset and count
     * @param lpWch The destination. the provided nullptr will be ignored;
     * @param bufferSize destination buffer size
     * @param offset the offset that want copied first char
     * @param cchRequire require copied text size in chars
     */
    auto UnsafeGetText(LPWCH lpWch, size_t bufferSize, size_t offset, size_t cchRequire) const -> void;

    [[nodiscard]] constexpr auto GetText() const -> std::wstring
    {
        const std::shared_lock lock(m_mutex);
        return m_editorText;
    }

private:
    TS_SELECTION_ACP m_acpSelection{
        .acpStart = 0, .acpEnd = 0, .style = {.ase = TS_AE_END, .fInterimChar = FALSE}
    };
    mutable std::shared_mutex m_mutex;
    std::wstring              m_editorText;
};
} // namespace Ime

#endif
