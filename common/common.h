#pragma once

#ifdef __clang__
    // Declare a LIBC_NAMESPACE with hidden visibility. `namespace
    // LIBC_NAMESPACE_DECL {` should be used around all declarations and definitions
    // for libc internals as opposed to just `namespace LIBC_NAMESPACE {`. This
    // ensures that all declarations within this namespace have hidden
    // visibility, which optimizes codegen for uses of symbols defined in other
    // translation units in ways that can be necessary for correctness by avoiding
    // dynamic relocations. This does not affect the public C symbols which are
    // controlled independently via `LLVM_LIBC_FUNCTION_ATTR`.
    #define LIBC_NAMESPACE_DECL [[gnu::visibility("hidden")]] LIBC_NAMESPACE
#else
    // needs to be diagnosed and addressed.
    #define LIBC_NAMESPACE_DECL LIBC_NAMESPACE
#endif

#ifdef __clang__
    #define ALIGN(size) __attribute__((packed)) __attribute__((aligned(size)))
#else
    #define ALIGN(size)
#endif

#ifdef LIBC_NAMESPACE_DECL

    #include <cstdint>
    #include <source_location>
    #include <spdlog/common.h>
    #include <spdlog/spdlog.h>

    #ifdef PLUGIN_NAMESPACE
        #define PLUGIN_NAMESPACE_FULL LIBC_NAMESPACE::PLUGIN_NAMESPACE
    #endif

    #ifndef COMMON_H
        #define COMMON_H

namespace LIBC_NAMESPACE_DECL
{
    struct format_string_loc
    {
        constexpr format_string_loc(const char                 *aFmt,
                                    const std::source_location &location = std::source_location::current()) noexcept
            : m_value{aFmt}, m_loc(spdlog::source_loc{location.file_name(), static_cast<std::int32_t>(location.line()),
                                                      location.function_name()})
        {
        }

        [[nodiscard]] constexpr auto GetValue() const noexcept -> const std::string_view &
        {
            return m_value;
        }

        [[nodiscard]] constexpr auto GetLoc() const noexcept -> const spdlog::source_loc &
        {
            return m_loc;
        }

    private:
        std::string_view   m_value;
        spdlog::source_loc m_loc;
    };

    template <typename EnumType, typename... Args>
    static constexpr auto logd(EnumType level, const format_string_loc &fsl, Args &&...args) noexcept
        requires(std::same_as<EnumType, spdlog::level::level_enum>)
    {
        auto fmt = std::vformat(fsl.GetValue(), std::make_format_args(args...));
        spdlog::log(fsl.GetLoc(), level, fmt.c_str());
    }

    template <typename... Args>
    static constexpr auto log_critical(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::critical, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_error(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::err, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_warn(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::warn, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_info(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::info, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_debug(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::debug, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_trace(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::trace, fsl, std::forward<Args>(args)...);
    }

    void InitializeLogging(spdlog::level::level_enum logLevel, spdlog::level::level_enum flushLevel);

    bool PluginInit();

    void InitializeMessaging();

    int  ErrorHandler(unsigned int code, _EXCEPTION_POINTERS *);

} // namespace LIBC_NAMESPACE_DECL

    #endif
#endif // LIBC_NAMESPACE_DECL
