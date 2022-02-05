### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

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
