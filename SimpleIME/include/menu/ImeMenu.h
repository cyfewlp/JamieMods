//
// Created by jamie on 2026/1/1.
//

#pragma once

#include "RE/B/BSUIScaleformData.h"
#include "RE/GFxCharEvent.h"
#include "RE/I/IMenu.h"
#include "common/config.h"

enum ImGuiKey : int;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ImeMenu final : public RE::IMenu
{
    bool m_fSShow = false;

public:
    static void RegisterMenu();

    void PostDisplay() override;

    auto ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS override;

    void OnShow();

    void OnHide();

private:
    static auto Creator() -> IMenu *;

    /// is handled?
    static auto ProcessScaleformEvent(const RE::BSUIScaleformData *data) -> RE::UI_MESSAGE_RESULTS;
    static auto OnKeyEvent(RE::GFxEvent *event, bool down) -> RE::UI_MESSAGE_RESULTS;
    static void SendKeyEventToImGui(const RE::GFxKeyEvent *keyEvent, bool down);
    static auto MapToImGuiKey(RE::GFxKey::Code keyCode) -> ImGuiKey;
    static auto OnMouseEvent(RE::GFxEvent *event, bool down) -> RE::UI_MESSAGE_RESULTS;
    static auto OnMouseWheelEvent(RE::GFxEvent *event) -> RE::UI_MESSAGE_RESULTS;
    static auto OnCharEvent(const RE::GFxCharEvent *charEvent) -> RE::UI_MESSAGE_RESULTS;

    static bool IsPaste(const RE::GFxCharEvent *charEvent);
    static bool Paste();
};
}
}
