### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.18)

option(DISABLE_VCPKG "Disable vcpkg" OFF)

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/../../..")
if (NOT DISABLE_VCPKG)
    set(CMAKE_TOOLCHAIN_FILE "${ROOT_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project(package_test_project
        LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BOOST_VERSION 1.74)
find_package(Boost ${BOOST_VERSION} REQUIRED)
include_directories(BEFORE SYSTEM ${Boost_INCLUDE_DIRS})

find_package(arg_router REQUIRED)
include_directories(BEFORE SYSTEM "${arg_router_INCLUDE_DIRS}")

add_executable(package_test_project
    "${ROOT_DIR}/examples/c++20/just_cats/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json")

target_compile_features(package_test_project PUBLIC cxx_std_20)
set_target_properties(package_test_project PROPERTIES CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(package_test_project PRIVATE /W4 /Z7)
    target_compile_definitions(package_test_project PRIVATE
        NOMINMAX
        BOOST_USE_WINDOWS_H
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )

    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        target_compile_options(package_test_project PRIVATE /clang:-fconstexpr-steps=10000000)
    endif()
else()
    target_compile_options(package_test_project PRIVATE
        -Werror
        -Wall
        -Wextra
        -ftemplate-backtrace-limit=0
    )
endif()
