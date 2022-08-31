### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

# Add a clang-tidy pass to the given target, if ENABLE_CLANG_TIDY is ON
function(add_clangtidy_to_target TARGET)
    if(ENABLE_CLANG_TIDY)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            message(STATUS "Enabling clang-tidy for ${TARGET}")
            set_target_properties(${TARGET} PROPERTIES CXX_CLANG_TIDY
                                  "clang-tidy;-p=${CMAKE_BINARY_DIR}")
        else()
            message(STATUS "Skipping clang-tidy as it is only for Clang compilers")
        endif()
    endif()
endfunction()
