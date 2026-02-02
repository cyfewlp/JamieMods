//
// Created by jamie on 2025/2/21.
//

#ifndef IME_ITEXTSERVICE_H
#define IME_ITEXTSERVICE_H

#include "TextEditor.h"
#include "core/State.h"

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

    virtual void UnInitialize() {}

    virtual void OnStart([[maybe_unused]] HWND hWnd) {}

    virtual bool OnFocus([[maybe_unused]] bool focus)
    {
        return true;
    }

    virtual auto ProcessImeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> bool = 0;
    virtual auto GetCandidateUi() -> CandidateUi &                                                = 0;
    virtual auto CommitCandidate(HWND hwnd, DWORD index) -> bool                                  = 0;
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

    /**
     * return true means message hav processed.
     */
    auto ProcessImeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> bool override;

    void OnStart(HWND hWnd) override
    {
        m_imeHwnd = hWnd;
    }

    bool OnFocus(bool focus) override;

    [[nodiscard]] auto GetCandidateUi() -> CandidateUi & override
    {
        return m_candidateUi;
    }

    auto CommitCandidate(HWND hwnd, DWORD index) -> bool override;

    [[nodiscard]] auto GetTextEditor() -> TextEditor & override
    {
        return m_textEditor;
    }

private:
    static void OnStartComposition();
    static void UpdateConversionMode(HIMC hIMC);
    void        OnEndComposition();
    void        OnComposition(HWND hWnd, LPARAM compFlag);
    void        UpdateComposition(const std::wstring &compStr, long cursorPos, long deltaStart);
    static auto GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, std::wstring &pWcharBuf) -> bool;
    auto        ImeNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) -> bool;

    void OpenCandidate(HIMC hIMC);
    void CloseCandidate();
    void ChangeCandidate(HIMC hIMC);
    void ChangeCandidateAt(HIMC hIMC);
    void DoUpdateCandidateList(LPCANDIDATELIST lpCandList);
    void OnSetOpenStatus(HIMC hIMC);

    HWND m_imeHwnd = nullptr;
    HIMC m_hIMC    = nullptr;

    CandidateUi m_candidateUi;
    TextEditor  m_textEditor;
};
} // namespace Imm32
} // namespace Ime

#endif // IME_ITEXTSERVICE_H
