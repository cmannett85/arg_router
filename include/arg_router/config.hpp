#pragma once

#include <cstddef>
#include <string_view>

namespace arg_router
{
/** Namespace for library basic types and constants i.e. compile-time
 * configuration.
 */
namespace config
{
/** Long form argument prefix. */
constexpr auto long_prefix = std::string_view{"--"};

/** Short form argument prefix. */
constexpr auto short_prefix = std::string_view{"-"};

static_assert(long_prefix.size() > short_prefix.size(),
              "Long prefix must be longer than short prefix");

/** Newline character sequence (platform dependent). */
#ifdef _WIN32
constexpr auto lf = std::string_view{"\r\n"};
#else
constexpr auto lf = std::string_view{"\n"};
#endif

/** Returns the L1 cache size.
 *
 * @return L1 cache size in bytes
 */
inline constexpr std::size_t l1_cache_size()
{
    // https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
#ifdef __cpp_lib_hardware_interference_size
    return std::hardware_destructive_interference_size;
#else
    return 2 * sizeof(std::max_align_t);
#endif
}
}  // namespace config
}  // namespace arg_router
