#pragma once

#include "common/common.h"
#include "ime/ime.h"

#include <TextStor.h>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class TextEditor
        {
        public:
            TextEditor()                                       = default;
            ~TextEditor()                                      = default;
            TextEditor(const TextEditor &other)                = delete;
            TextEditor(TextEditor &&other) noexcept            = delete;
            TextEditor &operator=(const TextEditor &other)     = delete;
            TextEditor &operator=(TextEditor &&other) noexcept = delete;

            auto        Select(const long &&acpStart, const long &&acpEnd) -> void;
            auto        Select(const TS_SELECTION_ACP *pSelectionAcp) -> void;

            auto        SelectAll()
            {
                m_acpSelection.acpStart = 0;
                m_acpSelection.acpEnd   = m_editorText.size();
            }

            void GetSelection(LONG *pAcpStart, LONG *pAcpEnd) const;
            auto GetSelection(TS_SELECTION_ACP *pSelectionAcp) const -> void;
            auto InsertText(const wchar_t *pwszText, const uint32_t cch) -> long;
            void ClearText();

            /**
             * @return The editor text size in characters
             */
            constexpr auto GetTextSize(__out long &charSize) const -> void
            {
                charSize = m_editorText.size();
            }

            /**
             * @return The editor text size in characters
             */
            constexpr auto GetTextSize() const -> uint32_t
            {
                return m_editorText.size();
            }

            /**
             * Require copied from the specified offset and count
             * @param lpWch The destination. the provided nullptr will be ignored;
             * @param bufferSize destination buffer size
             * @param offset the offset that want copied first char
             * @param cchRequire require copied text size in chars
             */
            auto GetText(LPWCH lpWch, uint32_t bufferSize, uint32_t offset, uint32_t cchRequire) const -> void;

            constexpr auto GetText() const -> const std::wstring &
            {
                return m_editorText;
            }

        private:
            TS_SELECTION_ACP m_acpSelection{
                .acpStart = 0, .acpEnd = 0, .style = {.ase = TS_AE_END, .fInterimChar = FALSE}
            };
            std::wstring m_editorText;
        };
    } // namespace Tsg
} // namespace LIBC_NAMESPACE_DECL