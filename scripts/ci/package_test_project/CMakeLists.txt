### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.18)

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/../../..")

project(package_test_project
        LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BOOST_VERSION 1.74)
find_package(Boost ${BOOST_VERSION} REQUIRED)
include_directories(BEFORE SYSTEM ${Boost_INCLUDE_DIRS})

find_package(arg_router REQUIRED)
include_directories(BEFORE SYSTEM "${arg_router_INCLUDE_DIRS}")

add_executable(package_test_project
    "${ROOT_DIR}/examples/c++20/just_cats/main.cpp")

target_compile_features(package_test_project PUBLIC cxx_std_20)
set_target_properties(package_test_project PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(package_test_project PRIVATE
    -Werror
    -Wall
    -Wextra
    -ftemplate-backtrace-limit=0
)
