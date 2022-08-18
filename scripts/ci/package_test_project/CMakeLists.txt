### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

cmake_minimum_required(VERSION 3.18)

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/../../..")
include("${ROOT_DIR}/cmake/config.cmake")

set(CMAKE_TOOLCHAIN_FILE "${ROOT_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(package_test_project
        LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(span-lite REQUIRED)
include_directories(BEFORE SYSTEM ${span-lite_INCLUDE_DIRS})

set(BOOST_VERSION 1.75)
find_package(Boost ${BOOST_VERSION} REQUIRED)
include_directories(BEFORE SYSTEM ${Boost_INCLUDE_DIRS})

find_package(arg_router REQUIRED)
include_directories(BEFORE SYSTEM "${arg_router_INCLUDE_DIRS}")
message(STATUS "arg_router: ${arg_router_INCLUDE_DIRS}")

add_executable(package_test_project
    "${ROOT_DIR}/examples/just_cats/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json")

target_compile_features(package_test_project PUBLIC cxx_std_17)
set_target_properties(package_test_project PROPERTIES CXX_EXTENSIONS OFF)
