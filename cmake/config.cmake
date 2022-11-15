### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

set(MAX_CTS_SIZE 128
    CACHE STRING "Maximum compile_time_string size, defaults to 128")
add_compile_definitions(AR_MAX_CTS_SIZE=${MAX_CTS_SIZE})

set(LONG_PREFIX "--"
    CACHE STRING "Long flag or argument prefix, defaults to '--'")
add_compile_definitions(AR_LONG_PREFIX="${LONG_PREFIX}")

set(SHORT_PREFIX "-"
    CACHE STRING "Short flag or argument prefix, defaults to '-'")
add_compile_definitions(AR_SHORT_PREFIX="${SHORT_PREFIX}")

set(ALLOCATOR "std::allocator"
    CACHE STRING "Container allocator, defaults to std::allocator")
add_compile_definitions(AR_ALLOCATOR=${ALLOCATOR})

set(UTF8_TRAILING_WINDOW_SIZE 16
    CACHE STRING "Trailing window size for grapheme cluster and line break algorithms, defaults to 16")
add_compile_definitions(AR_UTF8_TRAILING_WINDOW_SIZE=${UTF8_TRAILING_WINDOW_SIZE})

set(DISABLE_CPP20_STRINGS false
    CACHE STRING "Disable the use of C++20-style compile-time strings")
add_compile_definitions(AR_DISABLE_CPP20_STRINGS=${DISABLE_CPP20_STRINGS})
