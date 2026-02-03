//
// Created by jamie on 2026/1/17.
//
#pragma once

#include <random>

namespace ImeTest
{

class RandomUtils
{
    std::mt19937 gen;

public:
    explicit RandomUtils() : gen(std::random_device{}()) {}

    auto NextInt(int min, int max) -> int
    {
        return std::uniform_int_distribution<>(min, max)(gen);
    }

    auto NextBool() -> bool
    {
        return std::bernoulli_distribution(0.5)(gen);
    }

    auto NextFloat(float min, float max) -> float
    {
        return std::uniform_real_distribution(static_cast<double>(min), static_cast<double>(max))(gen);
    }

    auto NextStrinng(size_t length) -> std::string
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

} // namespace ImeTest
