### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.18)

project(conan_test_project
        LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(arg_router REQUIRED)

add_executable(conan_test_project
    "${CMAKE_SOURCE_DIR}/../../../examples/c++20/just_cats/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/conanfile.txt")

target_link_libraries(conan_test_project PUBLIC arg_router::arg_router)
target_compile_features(conan_test_project PUBLIC cxx_std_20)
set_target_properties(conan_test_project PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(conan_test_project PRIVATE
    -Werror
    -Wall
    -Wextra
    -ftemplate-backtrace-limit=0
)
