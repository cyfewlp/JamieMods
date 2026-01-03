//
// Created by jamie on 2026/1/1.
//

#pragma once

#include "RE/B/BSUIScaleformData.h"
#include "RE/GFxCharEvent.h"
#include "RE/I/IMenu.h"
#include "common/config.h"

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

    // is handled?
    static bool ProcessScaleformEvent(const RE::BSUIScaleformData *data);

    static bool OnKeyEvent(RE::GFxEvent *event, bool down);
    static bool OnMouseEvent(RE::GFxEvent *event, bool down);
    static bool OnMouseWheelEvent(RE::GFxEvent *event);
    static bool OnCharEvent(RE::GFxEvent *event);

    static bool IsPaste(const RE::GFxCharEvent *charEvent);
    static bool Paste();
};
}
}
