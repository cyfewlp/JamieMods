//
// Created by jamie on 2025/2/22.
//

#ifndef ITEXTSERVICEFACTORY_H
#define ITEXTSERVICEFACTORY_H

#pragma once

#include "ITextService.h"
#include "tsf/TextStore.h"

namespace Ime::TextServiceFactory
{
inline auto Create(const bool enableTsf) -> std::unique_ptr<ITextService>
{
    if (enableTsf)
    {
        const auto &tsfSupport = Tsf::TsfSupport::GetSingleton();
        if (auto textService = std::make_unique<Tsf::TextService>();
            SUCCEEDED(textService->Initialize(tsfSupport.GetThreadMgr(), tsfSupport.GetTfClientId())))
        {
            return textService;
        }
        logger::warn("Can't initialize Text Service with TSF. Fallback to IMM32.");
    }
    return std::make_unique<Imm32::Imm32TextService>();
}
} // namespace Ime::TextServiceFactory

#endif // ITEXTSERVICEFACTORY_H
