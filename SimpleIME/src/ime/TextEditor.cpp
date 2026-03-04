#include "ime/TextEditor.h"

#include <algorithm>

auto Ime::TextEditor::Select(int32_t acpStart, int32_t acpEnd) -> void
{
    m_acpSelection.acpStart = acpStart;
    m_acpSelection.acpEnd   = acpEnd;
}

auto Ime::TextEditor::Select(const TS_SELECTION_ACP *pSelectionAcp) -> void
{
    m_acpSelection.acpStart           = pSelectionAcp->acpStart;
    m_acpSelection.acpEnd             = pSelectionAcp->acpEnd;
    m_acpSelection.style.fInterimChar = pSelectionAcp->style.fInterimChar;
    if (m_acpSelection.style.fInterimChar != FALSE)
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
    if (m_acpSelection.style.fInterimChar != FALSE)
    {
        pSelectionAcp->style.ase = TS_AE_NONE;
    }
    else
    {
        pSelectionAcp->style.ase = m_acpSelection.style.ase;
    }
}

void Ime::TextEditor::GetClampedSelection(size_t &acpStart, size_t &acpEnd) const
{
    const auto textSize = m_editorText.size();

    // The acpStart/acpEnd may -1 which means the caret is at the end of text.
    // this cast will convert -1 to a very large number, but we will clamp it to text size later, so it is fine.
    acpStart = static_cast<size_t>(m_acpSelection.acpStart);
    acpEnd   = static_cast<size_t>(m_acpSelection.acpEnd);

    acpStart = std::clamp(acpStart, 0LLU, textSize);
    acpEnd   = std::clamp(acpEnd, 0LLU, textSize);
    acpEnd   = std::max(acpEnd, acpStart);
}

auto Ime::TextEditor::InsertText(const wchar_t *textBuffer, const size_t textBufferSize) -> bool
{
    if (textBuffer == nullptr && textBufferSize > 0)
    {
        return false;
    }

    const auto textSize = m_editorText.size();

    size_t start;
    size_t end;
    GetClampedSelection(start, end);

    if (start >= textSize)
    {
        m_editorText.append(textBuffer, textBufferSize);
    }
    else
    {
        m_editorText.replace(start, end - start, textBuffer, textBufferSize);
    }

    const auto acpNewEnd    = static_cast<int32_t>(start + textBufferSize); // positive, safe
    m_acpSelection.acpStart = acpNewEnd;
    m_acpSelection.acpEnd   = acpNewEnd;
    return true;
}
