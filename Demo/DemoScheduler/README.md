# DEMO SCHEDULER

## Building and Running
If you followed all the previous explained steps, now you are able to run the scheduler.
1. Open `main.c`, and set `mainCREATE_SIMPLE_BLINKY_DEMO_ONLY` to `1`.
2. Make sure that the macro `configUSE_APERIODIC_PREEMPTION` is set to 0 and the macro `configUSE_POLLING_SERVER` is set to 1. This avoid to use the preemption for the aperiodic tasks and allow you to use the implemented polling server to schedule the incoming tasks.
3. Open the terminal and digit the command `make --directory=build/gcc` to compile and then you are ready to run the demo. 
4. In order to use QEMU and run the demo, you have to digit the command `qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel build/gcc/output/RTOSDemo.out -monitor none -nographic -serial stdio`.<br>
If you are using VS Code, open the `demoScheduler.c`, select the “Run” button. Then select “Launch QEMU RTOSDemo” from the dropdown on the top right and press the play button. This will build and run the selected program.
