#pragma once

#include <ktl/chrono.hpp>
#include <ktl/fmt.hpp>
#include <ktl/string.hpp>
#include <ktl/string_view.hpp>
#include <ktl/type_traits.hpp>

namespace log
{
// Log level enum
enum class level
{
    debug,
    info,
    warning,
    error
};

namespace detail
{
// Get level name based on character type
template <typename CharT> constexpr auto get_level_name(level lvl) noexcept;

template <> constexpr auto get_level_name<char>(level lvl) noexcept
{
    switch (lvl)
    {
    case level::debug:
        return "DEBUG";
    case level::info:
        return "INFO";
    case level::warning:
        return "WARNING";
    case level::error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

template <> constexpr auto get_level_name<wchar_t>(level lvl) noexcept
{
    switch (lvl)
    {
    case level::debug:
        return L"DEBUG";
    case level::info:
        return L"INFO";
    case level::warning:
        return L"WARNING";
    case level::error:
        return L"ERROR";
    default:
        return L"UNKNOWN";
    }
}

TIME_FIELDS get_current_time() noexcept
{
    const auto    current_time{ktl::chrono::system_clock::now().time_since_epoch().count()};
    LARGE_INTEGER native_time;
    native_time.QuadPart = static_cast<long long>(current_time);
    TIME_FIELDS time_fields;
    RtlTimeToTimeFields(ktl::addressof(native_time), ktl::addressof(time_fields));
    return time_fields;
}

// Output to debug console
template <size_t N, typename... Types>
inline void log_to_debug_output(const char (&format)[N], const Types&... args) noexcept
{
    DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, format, args...);
}

template <size_t N, typename... Types>
inline void log_to_debug_output(const wchar_t (&format)[N], const Types&... args) noexcept
{
    DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, format, args...);
}

// Log implementation for string views
inline void log_impl(ktl::ansi_string_view msg) noexcept
{
    log_to_debug_output("%Z\n", msg.raw_str());
}

inline void log_impl(ktl::unicode_string_view msg) noexcept
{
    log_to_debug_output("%wZ\n", msg.raw_str());
}

// Generic logging function templated on character type
template <typename CharT, class... Types>
void log_with_level(level lvl, const CharT* format, const Types&... args)
{
    auto time = get_current_time();

    // Select string type based on character type
    using string_view_t =
        ktl::conditional_t<ktl::is_same_v<CharT, char>, ktl::ansi_string_view, ktl::unicode_string_view>;

    using string_t =
        ktl::conditional_t<ktl::is_same_v<CharT, char>, ktl::ansi_string_non_paged, ktl::unicode_string_non_paged>;

    // Format prefix with timestamp and level
    string_t buffer;
    if constexpr (ktl::is_same_v<CharT, char>)
    {
        ktl::format_to(
            ktl::back_inserter(buffer),
            "[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}] [{}] ",
            time.Year,
            time.Month,
            time.Day,
            time.Hour,
            time.Minute,
            time.Second,
            time.Milliseconds,
            get_level_name<CharT>(lvl)
        );
    }
    else
    {
        ktl::format_to(
            ktl::back_inserter(buffer),
            L"[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}] [{}] ",
            time.Year,
            time.Month,
            time.Day,
            time.Hour,
            time.Minute,
            time.Second,
            time.Milliseconds,
            get_level_name<CharT>(lvl)
        );
    }

    // Format the message
    string_t msg_part;
    ktl::format_to(ktl::back_inserter(msg_part), string_view_t(format), args...);

    // Append message to buffer
    if constexpr (ktl::is_same_v<CharT, char>)
    {
        ktl::format_to(ktl::back_inserter(buffer), "{}", msg_part);
    }
    else
    {
        ktl::format_to(ktl::back_inserter(buffer), L"{}", msg_part);
    }

    // Output the complete message
    log_impl(buffer);
}

} // namespace detail

// Public API - simplified with just four functions

// Debug level logging
template <typename CharT, class... Types> void debug(const CharT* format, const Types&... args)
{
    detail::log_with_level(level::debug, format, args...);
}

// Info level logging
template <typename CharT, class... Types> void info(const CharT* format, const Types&... args)
{
    detail::log_with_level(level::info, format, args...);
}

// Warning level logging
template <typename CharT, class... Types> void warning(const CharT* format, const Types&... args)
{
    detail::log_with_level(level::warning, format, args...);
}

// Error level logging
template <typename CharT, class... Types> void error(const CharT* format, const Types&... args)
{
    detail::log_with_level(level::error, format, args...);
}

} // namespace log