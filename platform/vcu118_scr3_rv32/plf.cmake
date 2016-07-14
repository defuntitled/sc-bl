set(MARCH rv32imc_zba_zbb_zbc_zbs)
set(MARCH_OPTIONAL zifencei zicsr)
set(MABI ilp32)
set(MCPU scr3-rv32)

set(QEMU_MACHINE scr3,ipic=on)
set(QEMU_SMP 1)
set(QEMU_CPU syntacore-scr3,zba=true,zbb=true,zbc=true,zbs=true)
