### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

include("${CMAKE_SOURCE_DIR}/cmake/build_types/ast.cmake")

message(STATUS "Configuring examples")
add_subdirectory(basic_cat)
add_subdirectory(custom_policy_and_node)
add_subdirectory(just_cats)
add_subdirectory(simple)