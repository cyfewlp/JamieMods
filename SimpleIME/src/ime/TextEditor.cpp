#include "ime/TextEditor.h"

auto Ime::TextEditor::Select(const long acpStart, const long acpEnd) -> void
{
    m_acpSelection.acpStart = acpStart;
    m_acpSelection.acpEnd   = acpEnd;
}

auto Ime::TextEditor::Select(const TS_SELECTION_ACP *pSelectionAcp) -> void
{
    m_acpSelection.acpStart           = pSelectionAcp->acpStart;
    m_acpSelection.acpEnd             = pSelectionAcp->acpEnd;
    m_acpSelection.style.fInterimChar = pSelectionAcp->style.fInterimChar;
    if (m_acpSelection.style.fInterimChar)
    {
        m_acpSelection.style.ase = TS_AE_NONE;
    }
    else
    {
        m_acpSelection.style.ase = pSelectionAcp->style.ase;
    }
}

void Ime::TextEditor::GetSelection(LONG *pAcpStart, LONG *pAcpEnd) const
{
    if (pAcpStart != nullptr)
    {
        *pAcpStart = m_acpSelection.acpStart;
    }
    if (pAcpEnd != nullptr)
    {
        *pAcpEnd = m_acpSelection.acpEnd;
    }
}

void Ime::TextEditor::GetSelection(TS_SELECTION_ACP *pSelectionAcp) const
{
    pSelectionAcp->acpStart           = m_acpSelection.acpStart;
    pSelectionAcp->acpEnd             = m_acpSelection.acpEnd;
    pSelectionAcp->style.fInterimChar = m_acpSelection.style.fInterimChar;
    if (m_acpSelection.style.fInterimChar)
    {
        pSelectionAcp->style.ase = TS_AE_NONE;
    }
    else
    {
        pSelectionAcp->style.ase = m_acpSelection.style.ase;
    }
}

auto Ime::TextEditor::InsertText(const wchar_t *pwszText, const uint32_t cch) -> long
{
    std::unique_lock lock(m_mutex);

    const long textSize = static_cast<long>(m_editorText.size());

    long start = m_acpSelection.acpStart;
    long end   = m_acpSelection.acpEnd;

    if (start < 0) start = 0;
    if (end < 0) end = 0;

    if (start > textSize) start = textSize;
    if (end > textSize) end = textSize;

    if (end < start) end = start;

    if (start >= textSize)
    {
        m_editorText.append(pwszText, cch);
    }
    else
    {
        m_editorText.replace(
            static_cast<std::wstring::size_type>(start),
            static_cast<std::wstring::size_type>(end - start),
            pwszText,
            cch
        );
    }

    const long newCaret     = start + static_cast<long>(cch);
    m_acpSelection.acpStart = newCaret;
    m_acpSelection.acpEnd   = newCaret;
    return m_acpSelection.acpEnd;
}

void Ime::TextEditor::ClearText()
{
    std::unique_lock lock(m_mutex);
    m_editorText.clear();
}

auto Ime::TextEditor::UnsafeGetText(
    LPWCH lpWch, const uint32_t bufferSize, const uint32_t offset, const uint32_t cchRequire
) const -> void
{
    if (lpWch != nullptr)
    {
        wcsncpy_s(lpWch, bufferSize, m_editorText.substr(offset).c_str(), cchRequire);
    }
}
