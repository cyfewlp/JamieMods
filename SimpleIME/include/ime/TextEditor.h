#ifndef IME_TEXTEDITOR_H
#define IME_TEXTEDITOR_H

#pragma once

#include <TextStor.h>
#include <algorithm>
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
    auto Select(int32_t acpStart, int32_t acpEnd) -> void;
    auto Select(const TS_SELECTION_ACP *pSelectionAcp) -> void;

    auto SelectAll()
    {
        m_acpSelection.acpStart = 0;
        m_acpSelection.acpEnd   = -1;
    }

    void GetSelection(LONG *pAcpStart, LONG *pAcpEnd) const;
    auto GetSelection(TS_SELECTION_ACP *pSelectionAcp) const -> void;

    auto GetStart() const -> size_t
    {
        const auto uStart = static_cast<size_t>(m_acpSelection.acpStart);
        return std::clamp(uStart, 0LLU, m_editorText.size());
    }

    void GetClampedSelection(size_t &acpStart, size_t &acpEnd) const;
    auto InsertText(const wchar_t *textBuffer, size_t textBufferSize) -> bool;

    auto InsertText(std::wstring_view wsTextView) -> bool { return InsertText(wsTextView.data(), wsTextView.size()); }

    void ClearText() { m_editorText.clear(); }

    /**
     * @return The editor text size in characters
     */
    [[nodiscard]] constexpr auto GetTextSize() const -> size_t { return m_editorText.size(); }

    [[nodiscard]] constexpr auto GetText() -> std::wstring & { return m_editorText; }

private:
    TS_SELECTION_ACP m_acpSelection{
        .acpStart = 0, .acpEnd = 0, .style = {.ase = TS_AE_END, .fInterimChar = FALSE}
    };
    std::wstring m_editorText;
};
} // namespace Ime

#endif
