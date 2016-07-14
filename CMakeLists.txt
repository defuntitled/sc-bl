# Copyright (C) 2023, Syntacore Ltd.
# All Rights Reserved.

cmake_minimum_required(VERSION 3.16)
include(${CMAKE_CURRENT_LIST_DIR}/version.cmake)

project(fsbl VERSION ${SCBL_VERSION}
        LANGUAGES C ASM
        DESCRIPTION "Syntacore RISC-V First Stage Bootloader"
        HOMEPAGE_URL https://syntacore.com/page/products/sw-tools)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_C_STANDARD 99)

find_program(DTC dtc REQUIRED)
find_program(SED sed REQUIRED)
find_program(DD dd REQUIRED)
find_package(Git)

option(DMA_COHERENT_ENABLED "Build SCBL with enabled coherent DMA" OFF)
option(SCBL_NO_AUTOSTART "Disable SCBL autostart" OFF)

# ld script lookup
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/${PLATFORM}.ld)
    set(HAL_PLATFORM_LD_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/${PLATFORM}.ld)
else()
    set(HAL_PLATFORM_LD_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/fsbl.ld)
endif()

# set hal default to use 2nd printf level
set(HAL_PRINTF_LEVEL "2" CACHE STRING "HAL printf level")

# WA. cmake-toolchains set PLATFORM_CONFIG_DIR as
# ${CMAKE_SOURCE_DIR}/platform/${PLATFORM} or as
# ${SC_HAL_PATH}/platform/${PLATFORM}
# So prevent using BSP platforms
if(NOT "${PLATFORM_CONFIG_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}/platform/${PLATFORM}")
    message(FATAL_ERROR "BSP platform (${PLATFORM}) is used. Please use SC-BL one")
endif()

if(EXISTS ${SC_HAL_PATH}/CMakeLists.txt)
    add_subdirectory(${SC_HAL_PATH} hal)
else()
    message(FATAL_ERROR "sc-bl doesn't support pre-built hal")
endif()

# Checking dependency versions

#if(hal_VERSION VERSION_LESS_EQUAL 1.0.0)
#    message(FATAL_ERROR "Too old scr-hal version.\n"
#            "Please update scr-hal to at least version 1.0.0")
#endif()

#if(cmake_toolchains_VERSION VERSION_LESS_EQUAL 1.0.0)
#    message(FATAL_ERROR "Too old cmake-toolchains version.\n"
#            "Please update cmake-toolchains to at least version 1.0.0")
#endif()

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}")

set(FSBL_ELF "$<TARGET_FILE:fsbl>")
set(FSBL_MAP ${OUTPUT_DIR}/fsbl.map)
set(FSBL_BIN ${OUTPUT_DIR}/fsbl.bin)
set(FSBL_DUMP ${OUTPUT_DIR}/fsbl.dump)
set(FSBL_MEM ${OUTPUT_DIR}/fsbl.mem)

add_executable(fsbl)

# FIXME: linker depens on the order the sources. If files are put in
# alphabetical order, linux hangs up on vcu118_scr9_l3, smp=2 (qemu)
target_sources(fsbl PRIVATE
    src/fsbl.c
    src/xmodem.c
    src/reg_detect.c
    src/minit.c
    src/mtrap.c
    src/emu_ldst.c
    src/spinlock.c
    src/ipi.c
    src/cluster_control.c
    src/pmu.c
    src/pmu_events.c
    src/l2_pmu.c
    src/l3_pmu.c
    # src/mmc.c
    # src/ocsdc.c
    # src/fs.c
    # src/fat.c
    src/con_utils.c
    src/kbd.c
    src/xqspi.c
    src/plic.c
    src/imsic.c
    src/aplic.c
    src/cpu_features.c
    src/mentry.S
    src/clk.c
)


# Expose hal headers with "hal" prefix: '#include <hal/some_header.h>'
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/hal_headers)
file(CREATE_LINK ${SC_HAL_PATH}/include ${CMAKE_CURRENT_BINARY_DIR}/hal_headers/hal SYMBOLIC)
target_include_directories(fsbl PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/hal_headers)

target_include_directories(fsbl PRIVATE include platform/common)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/memcheck memcheck)
add_dependencies(fsbl run_memcheck)

set(PLATFORM_DTS_FILE ${PLATFORM_CONFIG_DIR}/plf.dts)
if(EXISTS ${PLATFORM_DTS_FILE})
    set(hal_inc $<TARGET_PROPERTY:hal,INTERFACE_INCLUDE_DIRECTORIES>)
    add_custom_command(OUTPUT ${OUTPUT_DIR}/platform.dts ${CMAKE_CURRENT_BINARY_DIR}/platform_dts.S
        COMMAND ${CMAKE_C_COMPILER} -march=${MARCH} -mabi=${MABI}
            -DPLATFORM=${PLATFORM} -nostdinc -E -P -x assembler-with-cpp
            "-D$<JOIN:$<TARGET_PROPERTY:fsbl,COMPILE_DEFINITIONS>,;-D>"
            "-I$<JOIN:$<TARGET_PROPERTY:fsbl,INCLUDE_DIRECTORIES>,;-I>"
            "$<$<BOOL:${hal_inc}>:-I$<JOIN:${hal_inc},;-I>>"
            -o ${OUTPUT_DIR}/platform.dts ${PLATFORM_DTS_FILE}
        COMMAND ${DTC} -O asm -o ${CMAKE_CURRENT_BINARY_DIR}/platform_dts.S
            ${OUTPUT_DIR}/platform.dts
        COMMAND ${SED} -i '1s/^/.section .dtb.rodata,"a"\\n/'
            ${CMAKE_CURRENT_BINARY_DIR}/platform_dts.S
        DEPENDS ${SED} ${DTC} ${PLATFORM_DTS_FILE}
        COMMAND_EXPAND_LISTS)
    target_compile_definitions(fsbl PRIVATE PLF_DTS_FILE=1)
    target_sources(fsbl PRIVATE platform_dts.S)
else()
  message(WARNING "No dts file found")
endif()

target_compile_options(fsbl PRIVATE
    -Os -Wall -Werror -Wno-error=cpp
    -mstrict-align
    -ffast-math
    -fomit-frame-pointer
    -fno-exceptions
    -ftls-model=local-exec
    -fno-asynchronous-unwind-tables
    -fno-unwind-tables
    -fdata-sections -ffunction-sections
    -fno-common
    -fno-builtin-printf)

target_link_libraries(fsbl PRIVATE hal -lgcc -lc)

target_link_options(fsbl PRIVATE
    -static -nostdlib -nostartfiles -ffreestanding
    "LINKER:--gc-sections,-Map=${FSBL_MAP}")

if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

    execute_process(COMMAND ${GIT_EXECUTABLE} diff-index --name-only HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_UNCOMMITED
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

    if(GIT_UNCOMMITED)
        set(GIT_TAG "${GIT_TAG}-dirty")
    endif()

    target_compile_definitions(fsbl PRIVATE FW_VER_CFG=\"g${GIT_TAG}\")
endif()

# cmake ... -DVERSION_TAG=1.0
if(VERSION_TAG)
    target_compile_definitions(fsbl PRIVATE VERSION_TAG=${VERSION_TAG})
endif()

if(DMA_COHERENT_ENABLED)
    target_compile_definitions(fsbl PRIVATE PLF_DMA_COHERENT=1)
endif()

if(SCBL_NO_AUTOSTART)
    target_compile_definitions(fsbl PRIVATE PLF_NO_AUTOSTART=1)
endif()

target_compile_definitions(fsbl PRIVATE PLATFORM=${PLATFORM})

# create fsbl.bin & fsbl.dump
add_custom_command(TARGET fsbl POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -w -x -s -S ${FSBL_ELF} > ${FSBL_DUMP}
    COMMAND ${CMAKE_OBJCOPY} -S -O binary ${FSBL_ELF} ${FSBL_BIN}
    BYPRODUCTS ${FSBL_BIN} ${FSBL_DUMP})

# hex target (Xilinx)
add_custom_target(hex DEPENDS fsbl
    COMMAND ${CMAKE_COMMAND} -E echo "@00000000" > ${FSBL_MEM}
    COMMAND hexdump -v -e '4/1 \"%02x\" \"\\n\"' ${FSBL_BIN} >> ${FSBL_MEM})

# dist target
string(TIMESTAMP TS "%y%m%dT%H%M")
add_custom_target(dist DEPENDS fsbl hex
    COMMAND echo Repo: > info.txt
    COMMAND git config --get remote.origin.url >> info.txt
    COMMAND echo Branch: >> info.txt
    COMMAND git branch --show-current >> info.txt
    COMMAND echo Commit: >> info.txt
    COMMAND git rev-parse --short HEAD 2>/dev/null >> info.txt
    COMMAND echo Tag: ${GIT_TAG} >> info.txt
    COMMAND zip -9 -r -j -o ${OUTPUT_DIR}/scbl-${PLATFORM}.${TS}.zip
    ${FSBL_BIN} ${FSBL_ELF} ${FSBL_DUMP} ${FSBL_MEM} ${FSBL_MAP} info.txt)

add_custom_command(OUTPUT dummy.bin
    COMMAND ${DD} if=/dev/zero of=dummy.bin bs=1 count=1 >/dev/null 2>/dev/null)

set(PLF_MASTER_HART "0" CACHE STRING "Boot hart ID")
target_compile_definitions(fsbl PUBLIC PLF_MASTER_HART=${PLF_MASTER_HART})

# run and qemu-test targets
if(QEMU_MACHINE)
    if((${CMAKE_SYSTEM_PROCESSOR} MATCHES "riscv32") AND (DEFINED ENV{SC_QEMU_SYSTEM32}))
        set(QEMU $ENV{SC_QEMU_SYSTEM32})
    elseif((${CMAKE_SYSTEM_PROCESSOR} MATCHES "riscv64") AND (DEFINED ENV{SC_QEMU_SYSTEM64}))
        set(QEMU $ENV{SC_QEMU_SYSTEM64})
    else()
        set(QEMU qemu-system-${CMAKE_SYSTEM_PROCESSOR})
    endif()
    list(APPEND SC_QEMU_ARGS
        -M ${QEMU_MACHINE},close-at-exit=true
        -nographic
        -bios ${FSBL_ELF})
    if(DEFINED QEMU_CPU)
        list(APPEND SC_QEMU_ARGS -cpu ${QEMU_CPU})
    endif()

    # Look like,  the original QEMU_SMP logic uses env variable $QEMU_SMP,
    # so let check it first
    if(DEFINED ENV{QEMU_SMP})
        list(APPEND SC_QEMU_ARGS --smp ENV{QEMU_SMP})
    elseif(DEFINED QEMU_SMP)
        # otherwise, use value from plf.cmake
        list(APPEND SC_QEMU_ARGS --smp ${QEMU_SMP})
    endif()

    add_custom_target(run DEPENDS fsbl
        COMMAND ${QEMU} ${SC_QEMU_ARGS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)
    add_custom_target(qemu-test DEPENDS fsbl dummy.bin
        COMMAND
            TEST_BACKEND=qemu
            QEMU=${QEMU}
            QEMU_MACHINE=${QEMU_MACHINE}
            QEMU_CPU=${QEMU_CPU}
            QEMU_MEMORY=${QEMU_MEMORY}
            QEMU_SMP=${QEMU_SMP}
            QEMU_MASTER_HART=${PLF_MASTER_HART}
            LINUX=${CMAKE_CURRENT_BINARY_DIR}/dummy.bin
            SCBL=${CMAKE_CURRENT_BINARY_DIR}/fsbl.elf
            PLF=${PLATFORM}
            LOGS=${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/pytest_wrapper.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
        USES_TERMINAL)
else()
    message(STATUS "QEMU_MACHINE is not defined by platform, skipping run and qemu-test targets")
endif()


if(NOT DEFINED SCR_MEMORY_MAP)
    set(SCR_MEMORY_MAP syntacore)
endif()

# fpga-test target
add_custom_target(fpga-test DEPENDS fsbl dummy.bin
    COMMAND
        TEST_BACKEND=fpga
        PLF=${PLATFORM}
        LINUX=${CMAKE_CURRENT_BINARY_DIR}/dummy.bin
        SCBL=${CMAKE_CURRENT_BINARY_DIR}/fsbl.bin
        LOGS=${CMAKE_CURRENT_BINARY_DIR}
        MEMORY_MAP=${SCR_MEMORY_MAP}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/pytest_wrapper.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
    USES_TERMINAL)
