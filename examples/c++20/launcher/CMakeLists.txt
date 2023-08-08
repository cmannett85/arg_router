### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS
    filesystem
)

create_clangformat_target(
    NAME clangformat_example_launcher_cpp20
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_launcher_cpp20 "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_launcher_cpp20 clangformat_example_launcher_cpp20)

target_compile_features(example_launcher_cpp20 PUBLIC cxx_std_20)
set_target_properties(example_launcher_cpp20 PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_launcher_cpp20
    PRIVATE arg_router
    PUBLIC Boost::filesystem
)

configure_example_build(example_launcher_cpp20)
add_clangtidy_to_target(example_launcher_cpp20)

add_dependencies(cpp20_examples example_launcher_cpp20)
