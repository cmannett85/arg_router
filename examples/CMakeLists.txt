### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

include("${CMAKE_SOURCE_DIR}/cmake/build_types/ast.cmake")

function(configure_example_build TARGET)
    if (MSVC_FRONTEND)
        set(EXTRA_FLAGS /Zc:__cplusplus /W4 /Z7 /GR- /permissive- /bigobj /wd4996)

        # /MT by default as it simplifies the running of the examples
        set_property(TARGET ${TARGET} PROPERTY
                     MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(EXTRA_FLAGS -Werror -Wall -Wextra -fno-rtti -Wno-deprecated-declarations)
    endif()
    target_compile_options(${TARGET} PRIVATE ${EXTRA_FLAGS})
endfunction()

message(STATUS "Configuring examples")
add_subdirectory(c++17)

if(BUILD_CPP20_EXAMPLES)
    add_subdirectory(c++20)
endif()
