### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

set(SHORT_PREFIX "-"
    CACHE STRING "Sets the short flag or argument prefix, defaults to '-'"
)

set(LONG_PREFIX "--"
    CACHE STRING "Sets the long flag or argument prefix, defaults to '--'"
)

set(CONFIG_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/config.hpp
)

set(CONFIG_FOR_IDE
     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/config.cmake
     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/config.hpp.in
     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/config_script.cmake
)

set_source_files_properties(${CONFIG_FILE} PROPERTIES GENERATED TRUE)

add_custom_target(gen_config
    ALL
    COMMAND ${CMAKE_COMMAND} -DORIG_BASE_PATH=${CMAKE_CURRENT_SOURCE_DIR} -DSHORT_PREFIX=${SHORT_PREFIX} -DLONG_PREFIX=${LONG_PREFIX} -DCONFIG_FILE=${CONFIG_FILE} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/config_script.cmake
    SOURCES ${CONFIG_FOR_IDE}
)
add_dependencies(gen_config gen_version)
