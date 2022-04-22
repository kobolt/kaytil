cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

set(PROJECT_SOURCE_FILES
  ../main_pico.c
  ../console_pico.c
  ../disk_pico.c
  ../z80.c
  ../mem.c
  ../io.c
  )

set(PROJECT_CBIOS_HEX_FILE "../cbios.hex")
set(PROJECT_CPM22_HEX_FILE "../cpm22.hex")

set(PROJECT_IMG_FILES
  "disk_a.img"
  "disk_b.img"
  "disk_c.img"
  "disk_d.img"
  )

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cbios.tmp
  MAIN_DEPENDENCY ${PROJECT_CBIOS_HEX_FILE}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND srec_cat
  ARGS ${PROJECT_CBIOS_HEX_FILE} -intel -o ${CMAKE_CURRENT_BINARY_DIR}/cbios.tmp -binary
  )

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cpm22.tmp
  MAIN_DEPENDENCY ${PROJECT_CPM22_HEX_FILE}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND srec_cat
  ARGS ${PROJECT_CPM22_HEX_FILE} -intel -o ${CMAKE_CURRENT_BINARY_DIR}/cpm22.tmp -binary
  )

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cbios.bin
  MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/cbios.tmp
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND dd
  ARGS bs=1 skip=64000 if=${CMAKE_CURRENT_BINARY_DIR}/cbios.tmp of=${CMAKE_CURRENT_BINARY_DIR}/cbios.bin
  )

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cpm22.bin
  MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/cpm22.tmp
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND dd
  ARGS bs=1 skip=58368 if=${CMAKE_CURRENT_BINARY_DIR}/cpm22.tmp of=${CMAKE_CURRENT_BINARY_DIR}/cpm22.bin
  )

set(PROJECT_BIN_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/cbios.bin
  ${CMAKE_CURRENT_BINARY_DIR}/cpm22.bin
  )

set(PROJECT_BIN_OBJ_FILES "")
foreach(FILENAME ${PROJECT_BIN_FILES})
  get_filename_component(FILENAME_ONLY ${FILENAME} NAME)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o
                     MAIN_DEPENDENCY ${FILENAME}
                     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..
                     COMMAND ${CMAKE_OBJCOPY}
                     ARGS -I binary -O elf32-littlearm -B armv6s-m ${FILENAME_ONLY} ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o )
  list(APPEND PROJECT_BIN_OBJ_FILES ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o)
endforeach(FILENAME)

set(PROJECT_IMG_OBJ_FILES "")
foreach(FILENAME ${PROJECT_IMG_FILES})
  get_filename_component(FILENAME_ONLY ${FILENAME} NAME)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o
                     MAIN_DEPENDENCY ${FILENAME}
                     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                     COMMAND ${CMAKE_OBJCOPY}
                     ARGS -I binary -O elf32-littlearm -B armv6s-m --rename-section .data=.rodata,alloc,load,readonly,data,contents ${FILENAME_ONLY} ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o )
  list(APPEND PROJECT_IMG_OBJ_FILES ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o)
endforeach(FILENAME)

add_executable(kaytil
  ${PROJECT_SOURCE_FILES}
  ${PROJECT_BIN_OBJ_FILES}
  ${PROJECT_IMG_OBJ_FILES}
  )

target_compile_definitions(kaytil PRIVATE -DDISABLE_Z80_TRACE)

pico_enable_stdio_uart(kaytil 1)
pico_add_extra_outputs(kaytil)

target_link_libraries(kaytil pico_stdlib)

