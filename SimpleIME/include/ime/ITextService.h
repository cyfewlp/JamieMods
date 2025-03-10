//
// Created by jamie on 2025/2/21.
//

#ifndef IME_ITEXTSERVICE_H
#define IME_ITEXTSERVICE_H

#include "TextEditor.h"
#include "core/State.h"

#include "enumeration.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        using OnEndCompositionCallback = void(const std::wstring &compositionString);

        class ITextService
        {
            using State = Ime::Core::State;

        public:
            ITextService()                                                  = default;
            virtual ~ITextService()                                         = default;
            ITextService(const ITextService &other)                         = delete;
            ITextService(ITextService &&other) noexcept                     = delete;
            auto operator=(const ITextService &other) -> ITextService &     = delete;
            auto operator=(ITextService &&other) noexcept -> ITextService & = delete;

            virtual auto Initialize() -> HRESULT
            {
                return S_OK;
            }

            virtual void UnInitialize()
            {
            }

            // Enable of disable TextService;derived class must call parent Enable fun.
            virtual void Enable([[maybe_unused]] const bool enable)
            {
                if (enable)
                {
                    State::GetInstance()->Clear(State::IME_DISABLED);
                }
                else
                {
                    State::GetInstance()->Set(State::IME_DISABLED);
                }
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

        protected:
            OnEndCompositionCallback *m_OnEndCompositionCallback = nullptr;
        };

        namespace Imm32
        {
            class Imm32TextService final : public ITextService
            {
                using State = Ime::Core::State;

            public:
                Imm32TextService()                                                      = default;
                ~Imm32TextService() override                                            = default;
                Imm32TextService(const Imm32TextService &other)                         = delete;
                Imm32TextService(Imm32TextService &&other) noexcept                     = delete;
                auto operator=(const Imm32TextService &other) -> Imm32TextService &     = delete;
                auto operator=(Imm32TextService &&other) noexcept -> Imm32TextService & = delete;

                // silent ignore any IME message when disabled.
                void Enable(const bool enable) override
                {
                    isEnabled = enable;
                    ITextService::Enable(enable);
                }

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
                static auto OnStartComposition() -> HRESULT;
                static void UpdateConversionMode(HIMC hIMC);
                auto        OnEndComposition() -> HRESULT;
                auto        OnComposition(HWND hWnd, LPARAM compFlag) -> HRESULT;
                static auto GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, std::wstring &pWcharBuf) -> bool;
                auto        ImeNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) -> bool;

                void OpenCandidate(HIMC hIMC);
                void CloseCandidate();
                void ChangeCandidate(HIMC hIMC);
                void ChangeCandidateAt(HIMC hIMC);
                void DoUpdateCandidateList(LPCANDIDATELIST lpCandList);
                void OnSetOpenStatus(HIMC hIMC);

                CandidateUi m_candidateUi;
                TextEditor  m_textEditor;
                bool        isEnabled = true;
            };
        } // namespace Imm32
    } // namespace Ime
} // namespace LIBC_NAMESPACE_DECL

#endif // IME_ITEXTSERVICE_H
