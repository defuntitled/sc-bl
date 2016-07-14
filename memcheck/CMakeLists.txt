# Copyright (C) 2024, Syntacore Ltd.
# All Rights Reserved.

# Syntacore memory checker for MPU/PMP/MRT
cmake_minimum_required(VERSION 3.16)

project(memcheck VERSION ${SCBL_VERSION}
        LANGUAGES C
        DESCRIPTION "Syntacore memory checker for MPU/PMP/MRT"
        HOMEPAGE_URL https://syntacore.com/page/products/sw-tools)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)

find_program(HOSTCC cc REQUIRED)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/memcheck DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/memcheck.c
    COMMAND ${HOSTCC} -std=gnu99
        -DPLATFORM=${PLATFORM} "-D$<JOIN:$<TARGET_PROPERTY:fsbl,COMPILE_DEFINITIONS>,;-D>"
        -D__${CMAKE_SYSTEM_PROCESSOR}=1
        -I${PLATFORM_CONFIG_DIR} -I${CMAKE_SOURCE_DIR}/include
        -I${OUTPUT_DIR}/hal_headers -I${SC_HAL_PATH}/include
        -o ${CMAKE_CURRENT_BINARY_DIR}/memcheck ${CMAKE_CURRENT_SOURCE_DIR}/memcheck.c
    COMMAND_EXPAND_LISTS)

add_custom_target(run_memcheck DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/memcheck
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/memcheck)
