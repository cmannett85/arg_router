### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

function(__ar_translation_string in_string out_string is_cpp17)
    if (is_cpp17)
        set(${out_string} "S_(${in_string})" PARENT_SCOPE)
    else()
        set(${out_string} "str<${in_string}>" PARENT_SCOPE)
    endif()
endfunction()

function(__ar_translation_body_generator)
    set(option_args CPP17)
    set(single_value_args LANGUAGE SOURCE OUTPUT_VAR)
    cmake_parse_arguments(ARGS "${option_args}" "${single_value_args}" "" ${ARGN})

    __ar_translation_string("\"${ARGS_LANGUAGE}\"" language_str ${ARGS_CPP17})
    set(header "template <>
class translation<${language_str}>
{
public:
")
    set(footer "};\n")

    set(error_code_header "\n    using error_code_translations = std::tuple<\n")
    set(error_code_footer "    >;\n")

    # Write the header
    set(output_data "${${ARGS_OUTPUT_VAR}}${header}")
    set(error_code_section FALSE)

    # CMake ranges are inclusive for some reason, so we need to decrement the array length before
    # using in the for-loop
    file(STRINGS "${ARGS_SOURCE}" lines ENCODING UTF-8)
    list(LENGTH lines num_lines_over)
    math(EXPR num_lines "${num_lines_over}-1")

    foreach(line_index RANGE ${num_lines})
        list(GET lines ${line_index} line)

        # Skip comment lines
        string(SUBSTRING "${line}" 0 1 first_char)
        if ((line STREQUAL "") OR (first_char STREQUAL "#"))
            continue()
        endif()

        if (line STREQUAL "[error_code]")
            set(error_code_section TRUE)
            string(APPEND output_data "${error_code_header}")
            continue()
        endif()

        # Split the string into key and value
        string(FIND "${line}" " = " div_pos)
        if (div_pos EQUAL -1)
            message(FATAL_ERROR "Malformed line: ${line}")
        endif()

        string(SUBSTRING "${line}" 0 ${div_pos} key)
        math(EXPR value_index "${div_pos}+3")
        string(SUBSTRING "${line}" ${value_index} -1 value)

        __ar_translation_string("${value}" value_str ${ARGS_CPP17})

        # Process the entry data
        if (error_code_section)
            set(output_line "        std::pair<traits::integral_constant<error_code::${key}>, ${value_str}>")
            # Don't append a comma if this is the last tuple entry, won't compile in C++
            if (line_index EQUAL num_lines)
                string(APPEND output_line "\n")
            else()
                string(APPEND output_line ",\n")
            endif()
            string(APPEND output_data "${output_line}")
        else()
            set(output_line "    using ${key} = ${value_str};\n")
            string(APPEND output_data "${output_line}")
        endif()
    endforeach()

    # Write the footer and finish
    if (error_code_section)
        string(APPEND output_data "${error_code_footer}")
    endif()
    string(APPEND output_data "${footer}")

    set(${ARGS_OUTPUT_VAR} "${output_data}" PARENT_SCOPE)
endfunction()

function(arg_router_translation_generator)
    set(single_value_args OUTPUT_DIR GENERATED_FILES_VAR)
    set(multi_value_args SOURCES)
    cmake_parse_arguments(ARGS "" "${single_value_args}" "${multi_value_args}" ${ARGN})

    if (NOT DEFINED ARGS_SOURCES)
        message(FATAL_ERROR "Translation generator requires at least one source file")
    endif()
    if (NOT DEFINED ARGS_OUTPUT_DIR)
        message(FATAL_ERROR "Translation generator output directory must be set, typically CMAKE_CURRENT_BINARY_DIR")
    endif()
    if (NOT DEFINED ARGS_GENERATED_FILES_VAR)
        message(FATAL_ERROR "Translation generator requires a generated file output variable")
    endif()

    set(header "// Generated by CMake, do not modify manually
namespace arg_router::multi_lang
{
#ifdef AR_ENABLE_CPP20_STRINGS
")
    set(midder "#else\n")
    set(footer "#endif\n}  // namespace arg_router::multi_lang\n")

    foreach(source ${ARGS_SOURCES})
        get_filename_component(language_id "${source}" NAME_WLE)
        set(output_file "${ARGS_OUTPUT_DIR}/${language_id}.hpp")
        list(APPEND output_file_list "${output_file}")

        set(output_data "${header}")

        __ar_translation_body_generator(
            LANGUAGE "${language_id}"
            SOURCE "${source}"
            OUTPUT_VAR output_data)
        string(APPEND output_data "${midder}")

        __ar_translation_body_generator(
            LANGUAGE "${language_id}"
            SOURCE "${source}"
            OUTPUT_VAR output_data
            CPP17)
        string(APPEND output_data "${footer}")

        # Only write out the data if it differs from the existing
        if (EXISTS "${output_file}")
            file(READ "${output_file}" existing_data)
            if (NOT "${existing_data}" STREQUAL "${output_data}")
                message(STATUS "Generated ${language_id} translation file")
                file(WRITE "${output_file}" "${output_data}")
            endif()
        else()
            message(STATUS "Generated ${language_id} translation file")
            file(WRITE "${output_file}" "${output_data}")
        endif()
    endforeach()

    set(${ARGS_GENERATED_FILES_VAR} "${output_file_list}" PARENT_SCOPE)
endfunction()
