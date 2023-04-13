### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.18)

project(conan_test_project
        LANGUAGES CXX)

find_package(arg_router REQUIRED)

add_executable(conan_test_project "main.cpp")
target_link_libraries(conan_test_project PUBLIC arg_router::arg_router)
