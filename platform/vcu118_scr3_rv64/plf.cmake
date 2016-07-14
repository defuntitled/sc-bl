set(MARCH rv64imac_zba_zbb_zbc_zbs)
set(MARCH_OPTIONAL zifencei zicsr)
set(MABI lp64)
set(MCPU scr3-rv64)

set(QEMU_MACHINE scr3)
set(QEMU_SMP 4)
set(QEMU_CPU syntacore-scr3,zba=true,zbb=true,zbc=true,zbs=true)
