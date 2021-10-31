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
constexpr static auto long_prefix = std::string_view{"--"};

/** Short form argument prefix. */
constexpr static auto short_prefix = std::string_view{"-"};

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
