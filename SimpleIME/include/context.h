//
// Created by jamie on 2025/2/23.
//

#ifndef CONTEXT_H
#define CONTEXT_H

#pragma once

#include "common/config.h"

#include <atomic>
#include <memory>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class Context
        {
            static std::unique_ptr<Context> g_context;
            std::atomic_bool                m_isGameLoading;

        public:
            [[nodiscard]] bool IsGameLoading() const
            {
                return m_isGameLoading;
            }

            void SetIsGameLoading(const bool isGameLoading)
            {
                m_isGameLoading.store(isGameLoading);
            }

            static auto GetInstance() noexcept -> Context *;
        };
    }
}

#endif // CONTEXT_H
