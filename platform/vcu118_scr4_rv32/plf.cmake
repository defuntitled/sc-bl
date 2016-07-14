set(MARCH rv32imfdc_zba_zbb_zbc_zbs)
set(MARCH_OPTIONAL zifencei zicsr)
set(MABI ilp32d)
set(MCPU scr4-rv32)

set(QEMU_MACHINE scr4,ipic=on)
set(QEMU_SMP 1)
set(QEMU_CPU syntacore-scr4,zba=true,zbb=true,zbc=true,zbs=true)