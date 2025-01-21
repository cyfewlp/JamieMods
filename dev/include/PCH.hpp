
#include "spdlog/spdlog.h"

#define LOG(a_type, ...)                                                                                               \
    spdlog::log(spdlog::source_loc(__FILE__, __LINE__, __FUNCTION__), spdlog::level::a_type, __VA_ARGS__)
