### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.18)

option(DISABLE_VCPKG "Disable vcpkg" OFF)

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/../..")
if (NOT DISABLE_VCPKG)
    set(CMAKE_TOOLCHAIN_FILE "${ROOT_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project(package_test_project
        LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(arg_router REQUIRED)

arg_router_translation_generator(
    SOURCES "${ROOT_DIR}/examples/resources/simple_ml_gen/en_GB.toml"
            "${ROOT_DIR}/examples/resources/simple_ml_gen/fr.toml"
            "${ROOT_DIR}/examples/resources/simple_ml_gen/ja.toml"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/translations/"
    GENERATED_FILES_VAR translation_files
)

# Default to C++20
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD "20")
endif()

add_executable(package_test_project
    $<IF:$<STREQUAL:"${CMAKE_CXX_STANDARD}","17">,
        "${ROOT_DIR}/examples/c++17/simple_ml_gen/main.cpp" ,
        "${ROOT_DIR}/examples/c++20/simple_ml_gen/main.cpp" >
    ${translation_files}
    "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json")

target_link_libraries(package_test_project PUBLIC arg_router::arg_router)
set_target_properties(package_test_project PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(package_test_project PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(package_test_project PRIVATE /W4 /Z7 /GR- /permissive-)
else()
    target_compile_options(package_test_project PRIVATE -Werror -Wall -Wextra)
endif()
