//
// Created by jamie on 2025/2/22.
//

#ifndef ITEXTSERVICEFACTORY_H
#define ITEXTSERVICEFACTORY_H

#pragma once

#include "ITextService.h"
#include "tsf/TextStore.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ITextServiceFactory
        {
        public:
            static auto CreateInstance(bool enableTsf, ITextService **ppTextService) -> void
            {
                std::unique_ptr<ITextService> pTextService = nullptr;
                if (enableTsf)
                {
                    *ppTextService = new Tsf::TextService();
                }
                else
                {
                    *ppTextService = new Imm32::Imm32TextService();
                }
            }
        };
    }
}

#endif // ITEXTSERVICEFACTORY_H
