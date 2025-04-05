//
// Created by jamie on 2025/4/4.
//

#ifndef SOSGUIMENU_H
#define SOSGUIMENU_H

#pragma once

enum ImGuiKey : int;

namespace LIBC_NAMESPACE_DECL
{

    class SosGuiMenu : public RE::IMenu
    {
        bool m_fShow = false;

    public:
        static constexpr std::string_view MENU_NAME = "SosGuiMenu";
        static void                       RegisterMenu();

        void PostDisplay() override;
        void OnHide();
        auto ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS override;

        void ToggleShow();

        constexpr auto IsShowing() const -> bool
        {
            return m_fShow;
        }

    private:
        static auto Creator() -> IMenu *;

        void        ProcessScaleformEvent(RE::BSUIScaleformData *data);
        static void OnMouseEvent(RE::GFxEvent *event, bool down);
        static void OnMouseWheelEvent(RE::GFxEvent *event);
        static void OnKeyEvent(RE::GFxEvent *event, bool down);
        static void OnCharEvent(RE::GFxEvent *event);

        static auto GFxKeyToImGuiKey(RE::GFxKey::Code keyCode) -> ImGuiKey;
    };

}

#endif // SOSGUIMENU_H
