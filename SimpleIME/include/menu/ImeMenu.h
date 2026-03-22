//
// Created by jamie on 2026/1/1.
//

#pragma once

#include "RE/GFxCharEvent.h"

enum ImGuiKey : int;

namespace Ime
{
class ImeMenu final : public RE::IMenu
{
    std::vector<RE::GFxEvent *> m_imeCharEvents;
    bool                        m_fSShow   = false;
    bool                        m_ctrlDown = false;

public:
    static void RegisterMenu();

    void PostDisplay() override;

    auto ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS override;

private:
    void OnShow();
    void OnHide();
    /// is handled?
    auto ProcessScaleformEvent(const RE::BSUIScaleformData *data) -> RE::UI_MESSAGE_RESULTS;
    auto OnKeyEvent(RE::GFxEvent *event, bool down) -> RE::UI_MESSAGE_RESULTS;
    auto OnCharEvent(const GFxCharEvent *charEvent) -> RE::UI_MESSAGE_RESULTS;

    bool IsPaste(const GFxCharEvent *charEvent) const;
};
} // namespace Ime
