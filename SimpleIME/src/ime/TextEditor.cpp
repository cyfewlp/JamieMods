#include "ime/TextEditor.h"

auto LIBC_NAMESPACE::Ime::TextEditor::Select(const long &&acpStart, const long &&acpEnd) -> void
{
    m_acpSelection.acpStart = acpStart;
    m_acpSelection.acpEnd   = acpEnd;
}

auto LIBC_NAMESPACE::Ime::TextEditor::Select(const TS_SELECTION_ACP *pSelectionAcp) -> void
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

void LIBC_NAMESPACE::Ime::TextEditor::GetSelection(LONG *pAcpStart, LONG *pAcpEnd) const
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

void LIBC_NAMESPACE::Ime::TextEditor::GetSelection(TS_SELECTION_ACP *pSelectionAcp) const
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

auto LIBC_NAMESPACE::Ime::TextEditor::InsertText(const wchar_t *pwszText, const uint32_t cch) -> long
{
    if (m_acpSelection.acpStart >= static_cast<LONG>(m_editorText.size()))
    {
        m_editorText.append(pwszText, cch);
    }
    else
    {
        m_editorText.replace(m_acpSelection.acpStart, m_acpSelection.acpEnd - m_acpSelection.acpStart, pwszText, cch);
    }
    m_acpSelection.acpEnd = m_acpSelection.acpStart + static_cast<long>(cch);
    return m_acpSelection.acpEnd;
}

void LIBC_NAMESPACE::Ime::TextEditor::ClearText()
{
    m_editorText.clear();
}

auto LIBC_NAMESPACE::Ime::TextEditor::GetText(LPWCH lpWch, const uint32_t bufferSize, const uint32_t offset, const uint32_t cchRequire) const
    -> void
{
    if (lpWch != nullptr)
    {
        wcsncpy_s(lpWch, bufferSize, m_editorText.substr(offset).c_str(), cchRequire);
    }
}