# DEMO EXAMPLES

## Building and Running
If you followed all the previous explained steps, now you are able to run each demo in this folder.
1. Open `main.c`, and set the `mainSELECT_DEMO` macro to the value related to the demo that you want to run. The value associated to each demo are declared in the lines below.
2. Open the terminal and digit the command `make --directory=build/gcc` to compile and then you are ready to run the desired demo. 
3. In order to use QEMU and run the demo, you have to digit the command `qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel build/gcc/output/RTOSDemo.out -monitor none -nographic -serial stdio`.<br>
If you are using VS Code, select the “Run” button. Then select “Launch QEMU RTOSDemo” from the dropdown on the top right and press the play button. This will build and run the selected program.