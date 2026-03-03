//
// Created by jamie on 2025/2/21.
//

#ifndef IME_ITEXTSERVICE_H
#define IME_ITEXTSERVICE_H

#include "CandidateUi.h"
#include "TextEditor.h"
#include "core/State.h"

namespace Ime
{
using OnEndCompositionCallback = void(const std::wstring &compositionString);

class ITextService
{
public:
    enum class DirtyFlag : uint8_t
    {
        None               = 0U,      ///< All changes are clean, no need to update candidate UI.
        CandidateSelection = 1U << 1, ///< candidate selection changed.
        CandidateList      = 1U << 2, ///< candidate list changed, selection may also changed.
        All                = CandidateList | CandidateSelection
    };

    inline friend DirtyFlag operator|(DirtyFlag lhs, DirtyFlag rhs)
    {
        using Underlying = std::underlying_type_t<DirtyFlag>;
        return static_cast<DirtyFlag>(static_cast<Underlying>(lhs) | static_cast<Underlying>(rhs));
    }

    inline friend DirtyFlag &operator|=(DirtyFlag &lhs, DirtyFlag rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    ITextService()                                                  = default;
    virtual ~ITextService()                                         = default;
    ITextService(const ITextService &other)                         = delete;
    ITextService(ITextService &&other) noexcept                     = delete;
    auto operator=(const ITextService &other) -> ITextService &     = delete;
    auto operator=(ITextService &&other) noexcept -> ITextService & = delete;

    virtual auto Initialize() -> HRESULT { return S_OK; }

    virtual void UnInitialize() {}

    virtual void OnStart([[maybe_unused]] HWND hWnd) {}

    virtual auto OnFocus([[maybe_unused]] bool focus) -> bool { return true; }

    virtual auto ProcessImeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> bool = 0;

    // UI thread only. Called once per frame before rendering.
    auto UpdateCandidateUiIfDirty() -> void
    {
        const auto flag = m_dirtyFlag.exchange(DirtyFlag::None, std::memory_order_acq_rel);
        if (flag != DirtyFlag::None)
        {
            RequestUpdateCandidateUi(m_candidateUi, flag);
        }
    }

    auto GetCandidateUi() const -> const CandidateUi & { return m_candidateUi; }

    void MarkDirty(DirtyFlag dirtyFlag)
    {
        auto current = m_dirtyFlag.load(std::memory_order_relaxed);
        while (!m_dirtyFlag.compare_exchange_weak(current, current | dirtyFlag, std::memory_order_release, std::memory_order_relaxed))
        {
        }
    }

    virtual auto CommitCandidate(DWORD index) -> bool = 0;
    virtual auto GetTextEditor() -> TextEditor &      = 0;

    virtual void RegisterCallback(OnEndCompositionCallback *callback) { m_OnEndCompositionCallback = callback; }

private:
    CandidateUi            m_candidateUi;
    std::atomic<DirtyFlag> m_dirtyFlag{DirtyFlag::All};

protected:
    OnEndCompositionCallback *m_OnEndCompositionCallback = nullptr;

    virtual void RequestUpdateCandidateUi(CandidateUi &uiForRead, DirtyFlag flag) = 0;
};

namespace Imm32
{
class Imm32TextService final : public ITextService
{
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

    void OnStart(HWND hWnd) override { m_imeHwnd = hWnd; }

    bool OnFocus(bool focus) override;

    auto CommitCandidate(DWORD index) -> bool override;

    [[nodiscard]] auto GetTextEditor() -> TextEditor & override { return m_textEditor; }

protected:
    void RequestUpdateCandidateUi(CandidateUi &uiForRead, DirtyFlag /*flag*/) override
    {
        std::lock_guard lock(m_mutex);
        uiForRead.swap(m_candidateUi);
    }

private:
    static void OnStartComposition();
    void        OnEndComposition();
    void        OnComposition(HWND hWnd, LPARAM compFlag);
    void        UpdateComposition(const std::wstring &compStr, size_t cursorPos, size_t deltaStart);
    auto        OnImeNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) -> void;

    void OpenCandidate(HIMC hIMC);
    void ChangeCandidate(HIMC hIMC);
    void ChangeCandidateAt(HIMC hIMC);
    void DoUpdateCandidateList(LPCANDIDATELIST lpCandList);

    TextEditor                m_textEditor;
    CandidateUi               m_candidateUi;
    HWND                      m_imeHwnd = nullptr;
    HIMC                      m_hIMC    = nullptr;
    mutable std::shared_mutex m_mutex;
};
} // namespace Imm32
} // namespace Ime

#endif // IME_ITEXTSERVICE_H
