//
// Created by jamie on 2026/1/17.
//
#pragma once

#include "configs/configuration.h"

#include <random>

namespace ImeTest
{

class RandomUtils
{
    std::mt19937 gen;

public:
    explicit RandomUtils() : gen(std::random_device{}()) {}

    auto NextInt(int min, int max) -> int { return std::uniform_int_distribution<>(min, max)(gen); }

    auto NextBool() -> bool { return std::bernoulli_distribution(0.5)(gen); }

    auto NextFloat(float min, float max) -> float { return std::uniform_real_distribution(static_cast<double>(min), static_cast<double>(max))(gen); }

    auto NextString(size_t length) -> std::string
    {
        std::uniform_int_distribution<> dis('a', 'z');

        std::string s;
        for (size_t i = 0; i < length; ++i)
        {
            s += static_cast<char>(dis(gen));
        }
        return s;
    }
};

inline auto GetRandomConfiguation() -> Ime::Configuration
{
    ImeTest::RandomUtils random;
    Ime::Configuration   configuration{};
    configuration.shortcut                      = random.NextString(10);
    configuration.enableMod                     = random.NextBool();
    configuration.enableTsf                     = random.NextBool();
    configuration.fixInconsistentTextEntryCount = random.NextBool();

    configuration.logging.level      = random.NextString(10);
    configuration.logging.flushLevel = random.NextString(10);

    configuration.resources.translationDir = random.NextString(10);
    configuration.resources.fontPathList   = std::vector{random.NextString(10), random.NextString(10)};

    configuration.appearance.zoom                  = std::round(random.NextFloat(1.f, 9999.f) * 100.f) / 100.f;
    configuration.appearance.themeSourceColor      = random.NextInt(0, 0xffffff);
    configuration.appearance.themeDarkMode         = random.NextBool();
    configuration.appearance.themeContrastLevel    = 0.5;
    configuration.appearance.language              = random.NextString(10);
    configuration.appearance.errorDisplayDuration  = random.NextInt(0, 0xffff);
    configuration.appearance.verticalCandidateList = random.NextBool();

    configuration.input.enableUnicodePaste = random.NextBool();
    configuration.input.keepImeOpen        = random.NextBool();
    configuration.input.posUpdatePolicy    = random.NextString(10);
    return configuration;
}

} // namespace ImeTest
