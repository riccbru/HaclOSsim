#!/bin/zsh
cd ~/Desktop/Polito/CAOS/HaclOSsim/Demo/DemoScheduler/build/gcc
make clean
make
qemu-system-arm -s -S -M mps2-an385 -cpu cortex-m3 -monitor none -nographic -serial stdio -kernel ./output/RTOSDemo.out
