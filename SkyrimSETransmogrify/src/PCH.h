#pragma once

// This file is required.

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#define LOG(a_type, ...)                                                                                               \
    spdlog::log(spdlog::source_loc(__FILE__, __LINE__, __FUNCTION__), spdlog::level::a_type, __VA_ARGS__)

using namespace std::literals;

namespace Transmogrify {
};
using namespace Transmogrify;

template<typename T>
class Singleton
{
public:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    static auto GetSingleton() -> T&
    {
        std::call_once(initFlag, []() { instance.reset(new T); });
        return *instance;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;

private:
    static std::unique_ptr<T> instance;
    static std::once_flag initFlag;
};

// ��̬��Ա�����ĳ�ʼ��
template<typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

template<typename T>
std::once_flag Singleton<T>::initFlag;