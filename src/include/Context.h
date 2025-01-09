#pragma once

#include "RenderManager.h"
#include "input.h"

class Context final
{
private:
    Input::Devices* devices;
    Context() {};

public:
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;
    static inline std::atomic<bool> inputEventsEnabled = true;

    static auto& Singleton()
    {
        static Context context;
        return context;
    }

    void Init()
    {
        this->devices = new Input::Devices();
        RenderManager::Init();
    }

    Input::Devices* GetDevices() { return this->devices; }
};