#pragma once

#include "arg_router/utility/span.hpp"

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
}  // namespace config
}  // namespace arg_router
