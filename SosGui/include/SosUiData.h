//
// Created by jamie on 2025/4/3.
//

#ifndef SOSUIDATA_H
#define SOSUIDATA_H

#pragma once

#include "RE/A/Actor.h"
#include "common/config.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    class SosUiData
    {
        std::vector<RE::Actor *> m_actors;
        std::vector<RE::Actor *> m_NearActors;
        bool                     m_enabled;

    public:
        static auto GetInstance() -> SosUiData &
        {
            static SosUiData instance;
            return instance;
        }

        [[nodiscard]] auto GetActors() const -> const std::vector<RE::Actor *> &
        {
            return m_actors;
        }

        void SetActors(const std::vector<RE::Actor *> &actors)
        {
            m_actors.clear();
            for (const auto &actor : actors)
            {
                m_actors.push_back(actor);
            }
        }

        [[nodiscard]] auto GetNearActors() const -> const std::vector<RE::Actor *> &
        {
            return m_NearActors;
        }

        void SetNearActors(const std::vector<RE::Actor *> &nearActors)
        {
            m_NearActors.clear();
            for (const auto &actor : nearActors)
            {
                m_NearActors.push_back(actor);
            }
        }

        [[nodiscard]] auto IsEnabled() const -> bool
        {
            return m_enabled;
        }

        void SetEnabled(const bool enabled)
        {
            m_enabled = enabled;
        }
    };
}

#endif // SOSUIDATA_H
