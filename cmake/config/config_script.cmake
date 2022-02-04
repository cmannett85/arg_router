### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

set(CONFIG_FILE_TMP ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/config.hpp.tmp)

configure_file(
    ${ORIG_BASE_PATH}/cmake/config/config.hpp.in
    ${CONFIG_FILE_TMP}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CONFIG_FILE_TMP} ${CONFIG_FILE}
)
