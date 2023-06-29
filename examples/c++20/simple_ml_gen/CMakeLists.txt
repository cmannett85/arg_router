### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

include("${CMAKE_SOURCE_DIR}/cmake/translation_generator/translation_generator.cmake")
arg_router_translation_generator(
    SOURCES "${CMAKE_SOURCE_DIR}/examples/resources/simple_ml_gen/en_GB.toml"
            "${CMAKE_SOURCE_DIR}/examples/resources/simple_ml_gen/fr.toml"
            "${CMAKE_SOURCE_DIR}/examples/resources/simple_ml_gen/ja.toml"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/translations/"
    TARGET translation_example_simple_ml_gen_cpp20
    INTERNAL
)

create_clangformat_target(
    NAME clangformat_example_simple_ml_gen_cpp20
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_simple_ml_gen_cpp20
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(
    example_simple_ml_gen_cpp20
    translation_example_simple_ml_gen_cpp20
    clangformat_example_simple_ml_gen_cpp20)

target_include_directories(example_simple_ml_gen_cpp20 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_features(example_simple_ml_gen_cpp20 PUBLIC cxx_std_20)
set_target_properties(example_simple_ml_gen_cpp20 PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_simple_ml_gen_cpp20 PRIVATE arg_router)

configure_example_build(example_simple_ml_gen_cpp20)
add_clangtidy_to_target(example_simple_ml_gen_cpp20)

add_dependencies(cpp20_examples example_simple_ml_gen_cpp20)
