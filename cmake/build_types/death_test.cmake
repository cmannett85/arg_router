### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

path_prefixer(DEATH_TEST_DIR
    death_test
)
file(MAKE_DIRECTORY ${DEATH_TEST_DIR})

function(create_death_test NUM)
    # Make the death_test main stub to appease CMake, it'll get destroyed on first use
    set(DEATH_TEST_SRC
        "${DEATH_TEST_DIR}/main_${NUM}.cpp"
    )
    file(TOUCH ${DEATH_TEST_SRC})

    set(TARGET_NAME "arg_router_death_test_${NUM}")
    add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL ${DEATH_TEST_SRC})

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_EXTENSIONS OFF)
    target_compile_definitions(${TARGET_NAME} PRIVATE DEATH_TEST_BUILD)

    configure_test_build(${TARGET_NAME})

    target_include_directories(${TARGET_NAME}
        PUBLIC "${CMAKE_SOURCE_DIR}/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endfunction()

math(EXPR STOP_INDEX "${DEATH_TEST_PARALLEL}-1")
foreach(NUM RANGE ${STOP_INDEX})
    create_death_test(${NUM})
endforeach()
