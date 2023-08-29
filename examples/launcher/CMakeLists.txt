### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS
    filesystem
)

create_clangformat_target(
    NAME clangformat_example_launcher
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_launcher "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_launcher clangformat_example_launcher)

target_compile_features(example_launcher PUBLIC cxx_std_20)
set_target_properties(example_launcher PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_launcher
    PRIVATE arg_router
    PUBLIC Boost::filesystem
)

configure_example_build(example_launcher)
add_clangtidy_to_target(example_launcher)

add_dependencies(examples example_launcher)
