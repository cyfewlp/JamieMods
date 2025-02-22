#ifndef IME_TEXTEDITOR_H
#define IME_TEXTEDITOR_H

#pragma once

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
            TextEditor()                                                = default;
            ~TextEditor()                                               = default;
            TextEditor(const TextEditor &other)                         = delete;
            TextEditor(TextEditor &&other) noexcept                     = delete;
            auto operator=(const TextEditor &other) -> TextEditor &     = delete;
            auto operator=(TextEditor &&other) noexcept -> TextEditor & = delete;

            auto Select(const long &&acpStart, const long &&acpEnd) -> void;
            auto Select(const TS_SELECTION_ACP *pSelectionAcp) -> void;

            auto SelectAll()
            {
                std::shared_lock lock(m_mutex);
                m_acpSelection.acpStart = 0;
                m_acpSelection.acpEnd   = m_editorText.size();
            }

            void GetSelection(LONG *pAcpStart, LONG *pAcpEnd) const;
            auto GetSelection(TS_SELECTION_ACP *pSelectionAcp) const -> void;
            auto InsertText(const wchar_t *pwszText, uint32_t cch) -> long;
            void ClearText();

            /**
             * @return The editor text size in characters
             */
            constexpr auto GetTextSize(__out long &charSize) const -> void
            {
                std::shared_lock lock(m_mutex);
                charSize = m_editorText.size();
            }

            /**
             * @return The editor text size in characters
             */
            [[nodiscard]] constexpr auto GetTextSize() const -> uint32_t
            {
                std::shared_lock lock(m_mutex);
                return m_editorText.size();
            }

            /**
             * Require copied from the specified offset and count
             * @param lpWch The destination. the provided nullptr will be ignored;
             * @param bufferSize destination buffer size
             * @param offset the offset that want copied first char
             * @param cchRequire require copied text size in chars
             */
            auto UnsafeGetText(LPWCH lpWch, uint32_t bufferSize, uint32_t offset, uint32_t cchRequire) const -> void;

            [[nodiscard]] constexpr auto GetText() const -> std::wstring
            {
                std::shared_lock lock(m_mutex);
                return m_editorText;
            }

        private:
            TS_SELECTION_ACP m_acpSelection{
                .acpStart = 0, .acpEnd = 0, .style = {.ase = TS_AE_END, .fInterimChar = FALSE}
            };
            std::shared_mutex m_mutex;
            std::wstring      m_editorText;
        };
    } // namespace Ime
} // namespace LIBC_NAMESPACE_DECL
#endif
