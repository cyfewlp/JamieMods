#ifndef CONFIGS_CONVERTER_H
#define CONFIGS_CONVERTER_H

#pragma once

#include "Configs.h"

namespace PLUGIN_NAMESPACE
{
    template <typename D>
    struct converter
    {
    };

    template <>
    struct converter<float>
    {
        static constexpr float convert(const char *value, float aDefault = 0.0F)
        {
            if (value == nullptr)
            {
                return aDefault;
            }
            char       *pEnd{};
            const float result = std::strtof(value, &pEnd);
            if (*pEnd != 0)
            {
                return aDefault;
            }
            return result;
        }
    };

    template <>
    struct converter<int>
    {
        static constexpr int convert(const char *value, int aDefault = 0)
        {
            if (value == nullptr)
            {
                return aDefault;
            }
            std::string_view strView(value);
            int              radix = strView.starts_with("0x") ? 16 : 10;
            char            *pEnd{};
            int              result = std::strtol(value, &pEnd, radix);
            if (*pEnd != 0)
            {
                return aDefault;
            }
            return result;
        }
    };

    template <>
    struct converter<spdlog::level::level_enum>
    {
        static constexpr spdlog::level::level_enum convert(const char               *value,
                                                           spdlog::level::level_enum aDefault = spdlog::level::off)
        {
            if (value == nullptr)
            {
                return aDefault;
            }
            spdlog::level::level_enum result = aDefault;
            if (std::strcmp(value, "trace") == 0 || std::strcmp(value, "0") == 0)
            {
                result = spdlog::level::trace;
            }
            else if (std::strcmp(value, "debug") == 0 || std::strcmp(value, "1") == 0)
            {
                result = spdlog::level::debug;
            }
            else if (std::strcmp(value, "info") == 0 || std::strcmp(value, "2") == 0)
            {
                result = spdlog::level::info;
            }
            else if (std::strcmp(value, "error") == 0 || std::strcmp(value, "3") == 0)
            {
                result = spdlog::level::err;
            }
            else if (std::strcmp(value, "warn") == 0 || std::strcmp(value, "4") == 0)
            {
                result = spdlog::level::warn;
            }
            else if (std::strcmp(value, "critical") == 0 || std::strcmp(value, "5") == 0)
            {
                result = spdlog::level::critical;
            }
            else if (std::strcmp(value, "off") == 0 || std::strcmp(value, "6") == 0)
            {
                result = spdlog::level::off;
            }
            return result;
        }
    };

    template <>
    struct converter<uint32_t>
    {
        static constexpr uint32_t convert(const char *value, uint32_t aDefault = 0)
        {
            uint32_t result = aDefault;
            if (value == nullptr)
            {
                return result;
            }

            std::string_view strView(value);
            int              radix = strView.starts_with("0x") ? 16 : 10;
            char            *pEnd{};
            result = std::strtoul(value, &pEnd, radix);
            if (*pEnd != 0)
            {
                return aDefault;
            }
            return result;
        }
    };

    template <>
    struct converter<std::string>
    {
        static constexpr std::string convert(const char *value, const std::string &aDefault)
        {
            if (value == nullptr)
            {
                return aDefault;
            }

            return {value};
        }
    };

};

#endif // !CONFIGS_CONVERTER_H
