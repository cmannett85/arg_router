#pragma once

#if __cplusplus >= 202000L
#    include <span>
#else
#    define span_CONFIG_CONTRACT_LEVEL_OFF
#    include <nonstd/span.hpp>
#endif

namespace arg_router
{
namespace utility
{
#if __cplusplus >= 202000L
template <typename T, auto Extent = std::dynamic_extent>
using span = std::span<T, Extent>;
#else
template <typename T, auto Extent = nonstd::span_lite::dynamic_extent>
using span = nonstd::span_lite::span<T, Extent>;
#endif
}  // namespace utility
}  // namespace arg_router
