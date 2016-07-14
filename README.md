Syntacore RISC-V first stage bootloader
========================================

About
--------------

This package contains the Syntacore first stage bootloader (SCBL) for SCR SDK platforms.

Prerequisites
--------------

Syntacore Development Toolkit (SC-DT) is required to build SCBL.

#### Obtain SC-DT release

Pre-built RISC-V GCC toolchain and OpenOCD binaries are available to download from https://syntacore.com/tools/development-tools. Download the archive (*.tar.gz* for Linux, *.zip* for Windows) for your platform, extract the archive to your preferred directory and update the PATH environment variable as described in **Set environment variables** section.

Current SCBL version is compatible with sc-dt-2025.03.

#### Set environment variables

Add the <YOUR_INSTALL_PATH>/bin folder to the PATH environment variable:

    export PATH=$PATH:<YOUR_INSTALL_PATH>/bin

#### Before build

Several standard packages are needed to build the toolchain.
On Ubuntu, executing the following command should suffice:

    $ sudo apt-get install device-tree-compiler

Supported target platforms
--------------

| Target name           | Description           | Clock, MHz | mtimer clk source  | Cores | TCM   | Cache      | DRAM | OCRAM | Cluster | Memory mgmt    | Peripherals      | Arch | Comments                       |
| --------------------- | --------------------- | ---------- | ------------------ | ----- | ----- | ---------- | ---- | ----- | ------- | -------------- | ---------------- | -----| ------------------------------ |
| nexys_scr3_rv32       | Digilent Nexys 4 DDR  |            | internal (cluster) | SCR3  | 128K  | -          | 128M | 64K   |         | MPU            | IPIC             | rv32 | Supported boards: Digilent Arty A7-100T, Digilent Nexys 4 DDR |
| nexys_scr4_rv32       | Digilent Nexys 4 DDR  |            | internal (cluster) | SCR4  | 128K  | -          | 128M | 64K   |         | MPU            | IPIC             | rv32 | Supported boards: Digilent Arty A7-100T, Digilent Nexys 4 DDR |
|                       |                       |            |                    |       |       |            |      |       |         |                |                  |      |                                |
| vcu118_scr1           | AMD/Xilinx VCU118     | 90         | fixed              | SCR1  | 64K   | -          | 256K |       |         |                | IPIC             | rv32 |                                |
| vcu118_scr3_rv32      | AMD/Xilinx VCU118     |            | internal (cluster) | SCR3  | 128K  | L1         | 2G   | 64K   |         | MPU            | IPIC             | rv32 | IPIC with Vectored mode        |
| vcu118_scr3_rv64      | AMD/Xilinx VCU118     |            | internal (cluster) | SCR3  | 128K  | L1, L2     | 4G   | 64K   | L2      | MPU            | PLIC             | rv64 |                                |
| vcu118_scr4_rv32      | AMD/Xilinx VCU118     |            | internal (cluster) | SCR4  | 128K  | L1         | 1G   | 64K   |         | MPU            | IPIC             | rv32 | IPIC with Vectored mode        |
| vcu118_scr4_rv64      | AMD/Xilinx VCU118     |            | internal (cluster) | SCR4  | 128K  | L1, L2     | 4G   | 64K   | L2      | MPU            | PLIC             | rv64 |                                |
|                       |                       |            |                    |       |       |            |      |       |         |                |                  |      |                                |
| vcu118_scr5_rv32      | AMD/Xilinx VCU118     |            | internal (cluster) | SCR5  | 128K  | L1, L2     | 1G   | 64K   | L2      | MPU, MMU, SWPW | PLIC             | rv32 | TCM exists in single core version only |
| vcu118_scr5_rv64      | AMD/Xilinx VCU118     |            | internal (cluster) | SCR5  | 128K  | L1, L2     | 4G   | 64K   | L2      | MPU, MMU, SWPW | PLIC             | rv64 | TCM exists in single core version only |
|                       |                       |            |                    |       |       |            |      |       |         |                |                  |      |                                |
| vcu118_scr6           | AMD/Xilinx VCU118     |            | internal (cluster) | SCR6  |       | L1, L2     | 4G   | 64K   | L2      | MPU            | PLIC             | rv64 |                                |
|                       |                       |            |                    |       |       |            |      |       |         |                |                  |      |                                |
| vcu118_scr7_l2        | AMD/Xilinx VCU118     |            | internal (cluster) | SCR7  |       | L1, L2     | 4G   | 64K   | L2      | PMP, MMU       | PLIC, PMU, PCIE  | rv64 |                                |
| vcu118_scr7_l2_mpu    | AMD/Xilinx VCU118     |            | internal (cluster) | SCR7  |       | L1, L2     | 4G   | 64K   | L2      | MPU, MMU       | PLIC             | rv64 |                                |
|                       |                       |            |                    |       |       |            |      |       |         |                |                  |      |                                |
| vcu118_scr9_l2        | AMD/Xilinx VCU118     |            | internal (cluster) | SCR9  |       | L1, L2     | 4G   | 64K   | L2      | PMP, MMU       | PLIC, PMU        | rv64 |                                |
|                       |                       |            |                    |       |       |            |      |       |         |                |                  |      |                                |
| htg960_scr7_l2        | HiTech Global HTG960  |            | internal (cluster) | SCR7  |       | L1, L2     | 4G   | 64K   | L2      | PMP, MMU       | PLIC, PMU        | rv64 |                                |


Build SCBL
--------------

#### Install cmake-toolchains and scr-hal

*scr-hal* and *cmake-toolchains* are distributed as part of SCR SDK https://syntacore.com/tools/development-tools.

#### Prepare environment

Setup environment according SC-DT User guide. Usually, it is enough to `source /path/to/sc-dt/env.sh` and all necessary environment variables are set automatically.

Otherwise, environment variables may set manually:
* Set path to scr-hal: ```export SC_HAL_PATH=/path/to/scr-hal```. This is optional if *cmake-toolchains* and *scr-hal* reside in the same directory
* Set path to gcc: ```export SC_GCC_PATH=/path/to/riscv-gcc``` or ```export PATH=/path/to/riscv-gcc/bin/:$PATH```

#### Build SCBL

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=<path/to/toolchain_file.cmake> -DPLATFORM=<platform> ..
make
```

Supported toolchains:
- riscv64-elf-gcc.cmake
- riscv32-elf-gcc.cmake

The default toolchain path is `$SC_CMAKE_TOOLCHAINS`, usually it is `$SC_DT_PATH/workspace/cmake-toolchains`.

The toolchain must match the architecture of the platform, see the last column in the [list of supported platforms](#supported-target-platforms).
After the build process completes the target SCBL's files are created in the build directory

#### Run SCBL

By default, the command `make run` uses `$SC_QEMU_SYSTEM32` or `SC_QEMU_SYSTEM64` environment variables to run RV32 or RV64 simulator.
If you do not set `$SC_QEMU_SYSTEM32` or `SC_QEMU_SYSTEM64`, it is assumed that `qemu-system-riscv64` and `qemu-system-riscv32`
executables are available via ```$PATH```, you can run SCBL with the following shortcut command:

```bash
export PATH=/path/to/scr-qemu/bin:$PATH
make run
```


Notes
--------------

1. Master hart configuraton
SCBL allows booting not only from the hart with zero hart ID, but from any hart available on the system. By default,
zero hart ID is selected as the boot hart. If needed, that can be changed using environment variable PLF_MASTER_HART:

```bash
$ cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/riscv64-elf-gcc.cmake -DPLATFORM=vcu118_scr7_l2 -DPLF_MASTER_HART=2 ..
```

2. Disable autostart
SCBL autostart mode can be disabled via cmake option SCBL_NO_AUTOSTART
```bash
# build vcu118_scr9_l2 with no autostart:
$ cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/riscv64-elf-gcc.cmake -DPLATFORM=vcu118_scr9_l2 -DSCBL_NO_AUTOSTART=ON ..
```

3. Enable memory dump by 32-bit words
SC-BL allows to dump (print on via UART) memory content by reading 32-bit words (not bytes). This is useful if your peripheral
does **not** support byte access (for example, trace buffer). You should define `PLF_MEM_DUMP32=1` in command line or in `plf.h`
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to//riscv64-elf-gcc.cmake -DPLATFORM=<platform name> -DPLF_MEM_DUMP32=1 ..
```
