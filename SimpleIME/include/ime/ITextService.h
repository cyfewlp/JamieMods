//
// Created by jamie on 2025/2/21.
//

#ifndef ITEXTSERVICE_H
#define ITEXTSERVICE_H

#include "TextEditor.h"
#include "common/common.h"

#include "enumeration.h"
#include "ime.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        enum class ImeState : std::uint16_t
        {
            NONE             = 0,
            IN_COMPOSING     = 0x1,
            IN_CAND_CHOOSING = 0x2,
            IN_ALPHANUMERIC  = 0x4,
            IME_OPEN         = 0x8,
            ALL              = 0xFFFF
        };

        using OnEndCompositionCallback = void(const std::wstring &compositionString);

        class ITextService
        {
        public:
            ITextService()                                         = default;
            virtual ~ITextService()                                = default;
            ITextService(const ITextService &other)                = delete;
            ITextService(ITextService &&other) noexcept            = delete;
            ITextService &operator=(const ITextService &other)     = delete;
            ITextService &operator=(ITextService &&other) noexcept = delete;

            virtual auto  Initialize() -> HRESULT
            {
                return S_OK;
            }

            virtual void UnInitialize()
            {
            }

            virtual void OnStart([[maybe_unused]] HWND hWnd)
            {
            }

            virtual auto ProcessImeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> bool = 0;
            virtual auto GetCandidateUi() -> CandidateUi &                                                = 0;
            virtual auto GetTextEditor() -> TextEditor &                                                  = 0;

            virtual void RegisterCallback(OnEndCompositionCallback *callback)
            {
                m_OnEndCompositionCallback = callback;
            }

            // members
            auto GetState() const -> const Enumeration<ImeState> &
            {
                return m_state;
            }

            auto SetState(const ImeState &&state) -> void
            {
                m_state.set(state);
            }

            auto ClearState(const ImeState &&state) -> void
            {
                m_state.reset(state);
            }

        protected:
            Enumeration<ImeState>     m_state;
            OnEndCompositionCallback *m_OnEndCompositionCallback = nullptr;
        };

        namespace Imm32
        {
            class Imm32TextService final : public ITextService
            {
            public:
                Imm32TextService()                                             = default;
                ~Imm32TextService()                                            = default;
                Imm32TextService(const Imm32TextService &other)                = delete;
                Imm32TextService(Imm32TextService &&other) noexcept            = delete;
                Imm32TextService &operator=(const Imm32TextService &other)     = delete;
                Imm32TextService &operator=(Imm32TextService &&other) noexcept = delete;

                /**
                 * return true means message hav processed.
                 */
                auto ProcessImeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> bool override;

                [[nodiscard]] auto GetCandidateUi() -> CandidateUi & override
                {
                    return m_candidateUi;
                }

                [[nodiscard]] auto GetTextEditor() -> TextEditor & override
                {
                    return m_textEditor;
                }

            private:
                auto        OnStartComposition() -> HRESULT;
                auto        OnEndComposition() -> HRESULT;
                auto        OnComposition(HWND hWnd, LPARAM compFlag) -> HRESULT;
                static auto GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, std::wstring &pWcharBuf) -> bool;
                auto        ImeNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) -> bool;

                void        OpenCandidate(HIMC hIMC);
                void        CloseCandidate();
                void        ChangeCandidate(HIMC hIMC);
                void        ChangeCandidateAt(HIMC hIMC);
                void        DoUpdateCandidateList(LPCANDIDATELIST lpCandList);
                void        OnSetOpenStatus(HIMC hIMC);
                void        UpdateConversionMode(HIMC hIMC);

                CandidateUi m_candidateUi{};
                TextEditor  m_textEditor{};
            };
        }
    }
}

#endif // ITEXTSERVICE_H
