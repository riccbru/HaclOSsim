# group39 ~ HaclOSsim

## Tools Installation
### QEMU
- Arch
```bash
sudo yay -S qemu-full
```
Include both `qemu-base` for the version without GUI and `qemu-desktop` for the version with only x86_64 emulation by default.

- Ubuntu/Debian:
```bash
apt-get install qemu-system
```
- MacOS (Apple Silicon):
```bash
brew install qemu
```

### ARM Toolchain
- Arch
```bash
sudo yay -S arm-none-eabi-gcc 
```
- Ubuntu/Debian:
```bash
apt-get install gcc-arm-none-eabi
```
- MacOS
```bash
brew install armmbed/formulae/arm-none-eabi-gcc
```

### GNU Debugger GDB
- Arch
```bash
sudo yay -S arm-none-eabi-gdb
```
- Ubuntu/Debian:
```bash
apt-get install gdb-arm-none-eabi
```
- MacOS

To install `gdb-arm-none-eabi` go [here](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and look for `AArch32 bare-metal target (arm-none-eabi)`

### FreeRTOS
Either download it from [freertos.org](https://www.freertos.org/Documentation/02-Kernel/01-About-the-FreeRTOS-kernel/03-Download-freeRTOS/01-DownloadFreeRTOS) or via GitHub:
```bash
git clone https://github.com/FreeRTOS/FreeRTOS.git --recurse-submodules
```
The repo of interest is `FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC`.


### Running the FreeRTOS Demo 
#### From the terminal
- Launch `gdb`, type `set architecture arm` and quit.
- Go to the directory that contains the demo build with the command:
```bash
cd FreeRTOS/FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC/build/gcc
 ```
- Make sure that `Makefile` is present and build the demo with the command `make`.
- A file named `RTOSDemo.out` should have been generated in the `output` folder.
- Now QEMU can be run with the command:
```bash
qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel /path/to/RTOSDemo.out -monitor none -nographic -serial stdio -s -S
```
or (suitable for MacOS too):
```bash
sudo qemu-system-arm -M mps2-an385 -cpu cortex-m3 -nographic -kernel /path/to/RTOSDemo.out -s -S
```

Where `[path-to]` is the path to the `RTOSDemo.out` file previously generated.

N.B. Omit the "-s -S" options if you only want to run the FreeRTOS application in QEMU without attaching a debugger.
- Inside another terminal start the debugger with the command:
```bash
arm-none-eabi-gdb [path-to]/RTOSDemo.out
```
- Then connect to the target TCP port 1234:
```bash
(gdb) target remote :1234
```
- Finally, type the command `continue` to start the kernel:
```bash
(gdb) continue
```
You should now see the output displayed in the terminal where QEMU is running.
#### From Visual Studio Code
- Find the `gdb-arm-none-eabi` executable path using
```bash
which arm-none-eabi-gdb
```
- Open VSCode, find `.vscode/launch.json` and add JSON entry:
```json
"miDebuggerPath": "/path/to/gdb-arm-none-eabi"
```
