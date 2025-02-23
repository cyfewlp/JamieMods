#include "yaml-cpp/yaml.h"
#include <iostream>
#include <optional>

#include <stacktrace>
#include <spdlog/spdlog.h>

void funA()
{
    throw std::runtime_error("funA");
}

void funB()
{
    spdlog::info("start funB");
    funA();
}


int main()
{
    try
    {
        funB();
    } catch (const std::exception &e)
    {
        spdlog::error("Error {}", e.what());
        for (const auto & entry : std::stacktrace::current())
        {
            spdlog::error("   at:{}",  to_string(entry));
        }
    }
}