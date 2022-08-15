### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

path_prefixer(VERSION_FILE
    include/arg_router/version.hpp
)

path_prefixer(VERSION_FOR_IDE
     cmake/versioning/version.cmake
     cmake/versioning/version.hpp.in
     cmake/versioning/version_script.cmake
)

set_source_files_properties(${VERSION_FILE} PROPERTIES GENERATED TRUE)

add_custom_target(gen_version
    ALL
    COMMAND "${CMAKE_COMMAND}"
        -DORIG_BASE_PATH="${CMAKE_CURRENT_SOURCE_DIR}"
        -DPROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
        -DVERSION_FILE=${VERSION_FILE}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version_script.cmake"
    SOURCES ${VERSION_FOR_IDE}
)
