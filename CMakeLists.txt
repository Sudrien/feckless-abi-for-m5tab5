# feckless-abi-for-m5tab5
#
# The host's side: the feckless libraries, linked, and the table esp_elf
# resolves an app's ELF against, generated from exports.txt. The table
# names every symbol, which is also what keeps each one in the host's
# link: nothing in a launcher calls gfx_fill_star(), only an app might.
#
# The app's side is feckless_abi_elf(), in project_include.cmake: an app
# compiles against these headers and links none of it.
#
# SPDX-License-Identifier: MIT

set(_exports "${CMAKE_CURRENT_LIST_DIR}/exports.txt")
set(_table "${CMAKE_CURRENT_BINARY_DIR}/feckless_abi_syms.c")

idf_component_register(
    SRCS "src/feckless_abi.c" "${_table}"
    INCLUDE_DIRS "include"
    REQUIRES feckless_drivers feckless_graphics_handler
    PRIV_REQUIRES elf_loader esp_timer esp_app_format
)

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_exports}")
    file(STRINGS "${_exports}" _syms REGEX "^[A-Za-z_][A-Za-z0-9_]*$")
    # As elf_loader's tool/symbols.py writes it: only the address
    # matters, and "int" for memcpy needs the warning off.
    set(_c "/* Generated from exports.txt by CMakeLists.txt. Do not edit. */\n")
    string(APPEND _c "#include <stddef.h>\n#include \"private/elf_symbol.h\"\n")
    string(APPEND _c "#include \"feckless_abi.h\"\n\n")
    string(APPEND _c "#pragma GCC diagnostic push\n")
    string(APPEND _c "#pragma GCC diagnostic ignored \"-Wbuiltin-declaration-mismatch\"\n")
    foreach(_s IN LISTS _syms)
        string(APPEND _c "extern int ${_s};\n")
    endforeach()
    string(APPEND _c "#pragma GCC diagnostic pop\n\n")
    string(APPEND _c "const struct esp_elfsym g_feckless_abi_elfsyms[] = {\n")
    string(APPEND _c "    ESP_ELFSYM_EXPORT(FECKLESS_ABI_SYMBOL),\n")
    foreach(_s IN LISTS _syms)
        string(APPEND _c "    ESP_ELFSYM_EXPORT(${_s}),\n")
    endforeach()
    string(APPEND _c "    ESP_ELFSYM_END\n};\n")
    file(WRITE "${_table}.tmp" "${_c}")
    configure_file("${_table}.tmp" "${_table}" COPYONLY)
endif()
