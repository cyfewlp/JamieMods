//
// Created by jamie on 2025/2/21.
//

#ifndef INPUTMETHOD_H
#define INPUTMETHOD_H

#pragma once

#include "configs/Configs.h"

#include "TextEditor.h"
#include "enumeration.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class InputMethod
        {
        public:
            enum class State : std::uint16_t
            {
                NONE            = 0,
                IN_COMPOSITION  = 0x1,
                IN_CANDCHOOSEN  = 0x2,
                IN_ALPHANUMERIC = 0x4,
                OPEN            = 0x8,
                IME_ALL         = 0xFFFF
            };

            [[nodiscard]] constexpr auto GetComposition() const -> const std::wstring &
            {
                return m_textEditor->GetText();
            }

            [[nodiscard]] auto GetTextEditor() -> TextEditor *
            {
                return m_textEditor.get();
            }

            // State

            [[nodiscard]] constexpr auto GetState() const -> const Enumeration<State> &
            {
                return m_state;
            }

            auto SetState(const State &&state)
            {
                m_state.set(state);
            }

            auto ClearState(const State &&state)
            {
                m_state.reset(state);
            }

            // Candidate UI
            [[nodiscard]] constexpr auto GetCandidateUi() -> CandidateUi &
            {
                return m_candidateUi;
            }

            /**
             * Close current candidate list
             */
            void CloseCandidateUi()
            {
                m_candidateUi.Close();
            }

        private:
            std::unique_ptr<TextEditor> m_textEditor = std::make_unique<TextEditor>();
            CandidateUi                 m_candidateUi;
            Enumeration<State>          m_state;
        };
    }

} // LIBC_NAMESPACE_DECL

#endif // INPUTMETHOD_H
