### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

function(add_santizers_to_target TARGET)
    if(ENABLE_SANITIZERS)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            message(STATUS "Enabling santizers for ${TARGET}")  
            target_compile_options(${TARGET} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
                -fno-omit-frame-pointer
            )
            target_link_options(${TARGET} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
            )
        else()
            message(STATUS "Skipping sanitizers as they are only for Clang compilers")
        endif()
    endif()
endfunction()
