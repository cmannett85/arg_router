### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

function(configure_example_build TARGET)
    if(MSVC_FRONTEND)
        set(EXTRA_FLAGS /Zc:__cplusplus /W4 /Z7 /GR- /permissive- /bigobj /wd4996)

        # /MT by default as it simplifies the running of the examples
        set_property(TARGET ${TARGET} PROPERTY
                     MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(EXTRA_FLAGS ${EXTRA_FLAGS} /clang:-fconstexpr-steps=10000000)
        endif()
    else()
        set(EXTRA_FLAGS -Werror -Wall -Wextra -fno-rtti -Wno-deprecated-declarations)
    endif()
    target_compile_options(${TARGET} PRIVATE ${EXTRA_FLAGS})
endfunction()

message(STATUS "Configuring examples")
add_custom_target(examples ALL)
add_subdirectory(basic_cat)
add_subdirectory(custom_policy_and_node)
add_subdirectory(just_cats)
add_subdirectory(launcher)
add_subdirectory(runtime_node_enable)
add_subdirectory(simple)
add_subdirectory(simple_ml)
add_subdirectory(simple_ml_gen)
