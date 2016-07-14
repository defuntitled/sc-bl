set(MARCH rv64gc)
set(MARCH_OPTIONAL zba zbb zbc zbs)
set(MABI lp64d)
set(MTUNE scr9)

set(QEMU_MACHINE scr9_l2)
set(QEMU_CPU syntacore-scr9)
set(QEMU_SMP 4)
set(QEMU_MEMORY 4G)

if(DEFINED XGMAC_SUPPORT)
  add_compile_definitions(PLF_XGMAC_SUPPORT=${XGMAC_SUPPORT})
endif()
