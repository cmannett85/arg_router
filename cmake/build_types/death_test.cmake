### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

set(DEATH_TEST_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/death_test
)

set(DEATH_TEST_SRC
    ${DEATH_TEST_DIR}/main.cpp
)

# Make the death_test main stub to appease CMake, it'll get destroyed on first
# use.
file(MAKE_DIRECTORY ${DEATH_TEST_DIR})
file(TOUCH ${DEATH_TEST_SRC})

add_executable(arg_router_death_test EXCLUDE_FROM_ALL ${DEATH_TEST_SRC})
add_dependencies(arg_router_death_test arg_router)

target_compile_features(arg_router_death_test PUBLIC cxx_std_17)
set_target_properties(arg_router_death_test PROPERTIES CXX_EXTENSIONS OFF)

configure_test_build(arg_router_death_test)

target_include_directories(arg_router_death_test
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(arg_router_death_test
    PUBLIC arg_router
)
