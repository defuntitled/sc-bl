set(MARCH rv64imafdc_zba_zbb_zbc_zbs)
set(MARCH_OPTIONAL zifencei zicsr)
set(MABI lp64d)
set(MCPU scr4-rv64)

set(QEMU_MACHINE scr4)
set(QEMU_SMP 4)
set(QEMU_CPU syntacore-scr4,zba=true,zbb=true,zbc=true,zbs=true)
