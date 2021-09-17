set(CLANG_FORMAT_FILE
     ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format
)

add_custom_target(clangformat
    COMMAND clang-format --style=file --Werror -i ${MAIN_HEADERS} ${MAIN_SRCS} ${HEADERS} ${SRCS}
    SOURCES ${CLANG_FORMAT_FILE} ${MAIN_HEADERS} ${MAIN_SRCS} ${HEADERS} ${SRCS}
)
add_dependencies(clangformat gen_version)
