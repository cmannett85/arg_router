### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

# Add a clang-tidy pass to the given target, if ENABLE_CLANG_TIDY is ON
function(add_clangtidy_to_target TARGET)
    if(ENABLE_CLANG_TIDY)
        message(STATUS "Enabling clang-tidy for ${TARGET}")
        set_target_properties(${TARGET} PROPERTIES CXX_CLANG_TIDY
                              "clang-tidy;--format-style='file';--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
    endif()
endfunction()
