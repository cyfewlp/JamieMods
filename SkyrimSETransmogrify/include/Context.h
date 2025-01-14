#pragma once

#include "Config.h"
#include "RenderManager.h"

class Context final
{
private:
    Context() {};
    Config* config;

public:
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;

    static auto& Singleton()
    {
        static Context context;
        return context;
    }

    void Init();

    Config* LoadSettings(const char* path);
};