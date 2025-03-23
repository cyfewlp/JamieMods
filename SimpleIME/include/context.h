//
// Created by jamie on 2025/2/23.
//

#ifndef CONTEXT_H
#define CONTEXT_H

#pragma once

#include "common/config.h"

#include <memory>
#include <queue>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class Context
        {
            static std::unique_ptr<Context> g_context;
            std::queue<std::string>         m_message;
            std::atomic_bool                m_fKeepImeOpen = false;
            HWND                            m_hwndIme      = nullptr;

        public:
            [[nodiscard]] auto KeepImeOpen() const -> bool
            {
                return m_fKeepImeOpen;
            }

            void SetKeepImeOpen(const bool fKeepImeOpen)
            {
                m_fKeepImeOpen = fKeepImeOpen;
            }

            [[nodiscard]] auto Messages() -> std::queue<std::string> &
            {
                return m_message;
            }

            template <typename String>
            void PushMessage(const String &message)
            {
                m_message.push(message);
            }

            [[nodiscard]] auto HwndIme() const -> HWND
            {
                return m_hwndIme;
            }

            void SetHwndIme(const HWND hwndIme)
            {
                m_hwndIme = hwndIme;
            }

            static auto GetInstance() noexcept -> Context *;
        };
    }
}

#endif // CONTEXT_H
