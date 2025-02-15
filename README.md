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
qemu-system-arm -s -S -M mps2-an385 -cpu cortex-m3 -monitor none -nographic -serial stdio -kernel /path/to/RTOSDemo.out
```

N.B. Omit the "-s -S" options if you only want to run the FreeRTOS application in QEMU without attaching a debugger.
- Inside another terminal start the debugger with the command:
```bash
 arm-none-eabi-gdb [path-to]/RTOSDemo.out -ex "target remote :1234"
```
- Finally, type the command `continue` to start the kernel:
```bash
(gdb) continue
```
You should now see the output displayed in the terminal where QEMU is running.
#### From Visual Studio Code
- Find the `qemu-system-arm`, `arm-none-eabi-gcc`, `make`, `cmake` executable paths using:
```bash
which qemu-system-arm arm-none-eabi-gcc make cmake
```
- Add the QEMU, Arm GNU Compiler, CMake, and 'make' installation paths to your PATH environment variable.
- In VSCode, select 'File > Open Folder' in the menu and select this subfolder: '[path-to]/FreeRTOS/FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC'
- Open VSCode, find `.vscode/launch.json` and add JSON entry:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Launch QEMU RTOSDemo",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/gcc/output/RTOSDemo.out",
            "cwd": "${workspaceFolder}",
            "miDebuggerPath": "/Applications/ARM/bin/arm-none-eabi-gdb-py",
            "miDebuggerServerAddress": "localhost:1234",
            "stopAtEntry": true,
            "preLaunchTask": "Run QEMU"
        }
    ]
}
```
- Press the 'Run and Debug' button from the left side panel in VSCode. Select 'Launch QEMU RTOSDemo' from the dropdown at the top and press the 'play' button.
