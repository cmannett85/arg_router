### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

set(LONG_PREFIX "--"
    CACHE STRING "Long flag or argument prefix, defaults to '--'")
add_compile_definitions(AR_LONG_PREFIX="${LONG_PREFIX}")
message(STATUS "Setting long argument prefix to \'${LONG_PREFIX}\'")

set(SHORT_PREFIX "-"
    CACHE STRING "Short flag or argument prefix, defaults to '-'")
add_compile_definitions(AR_SHORT_PREFIX="${SHORT_PREFIX}")
message(STATUS "Setting short argument prefix to \'${SHORT_PREFIX}\'")

set(UTF8_TRAILING_WINDOW_SIZE 16
    CACHE STRING "Trailing window size for grapheme cluster and line break algorithms, defaults to 16")
add_compile_definitions(AR_UTF8_TRAILING_WINDOW_SIZE=${UTF8_TRAILING_WINDOW_SIZE})
message(STATUS "Setting trailing window size to \'${UTF8_TRAILING_WINDOW_SIZE}\'")

set(ENABLE_UTF8_SUPPORT ON
    CACHE BOOL "Enable UTF-8 support, defaults to ON")
add_compile_definitions(AR_ENABLE_UTF8_SUPPORT=$<BOOL:${ENABLE_UTF8_SUPPORT}>)
message(STATUS "Setting UTF-8 support to ${ENABLE_UTF8_SUPPORT}")
