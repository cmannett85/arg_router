### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.18)

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/../..")
set(CMAKE_TOOLCHAIN_FILE "${ROOT_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(vcpkg_test_project
        LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(arg_router REQUIRED)

add_executable(vcpkg_test_project
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json")
    
target_link_libraries(vcpkg_test_project PUBLIC arg_router::arg_router)
target_compile_features(vcpkg_test_project PUBLIC cxx_std_20)
set_target_properties(vcpkg_test_project PROPERTIES CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(vcpkg_test_project PRIVATE /W4 /Z7 /GR- /permissive-)
else()
    target_compile_options(vcpkg_test_project PRIVATE -Werror -Wall -Wextra)
endif()
