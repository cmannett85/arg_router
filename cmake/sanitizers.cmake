### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

function(add_santizers_to_target TARGET)
    if(ENABLE_SANITIZERS)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
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
