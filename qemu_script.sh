#!/bin/zsh

DEFAULT_DIR="$HOME/Desktop/Polito/CAOS/HaclOSsim/Demo/DemoScheduler/build/gcc"
PROJECT_DIR="${PROJECT_DIR:-$DEFAULT_DIR}"


if [ ! -d "$PROJECT_DIR" ]; then
    echo "Default path not found. Searching on Desktop..."
    SEARCH_PATH=$(find "$HOME/Desktop" -type d -name "HaclOSsim" -print -quit)
    
    if [ -n "$SEARCH_PATH" ]; then
        PROJECT_DIR="$SEARCH_PATH/Demo/DemoScheduler/build/gcc"
        echo "Found project directory at: $PROJECT_DIR"
    else
        echo "Error: Could not find HaclOSsim directory on Desktop."
        exit 1
    fi
fi

cd "$PROJECT_DIR" || { echo "Directory not found: $PROJECT_DIR"; exit 1; }

make clean
make
qemu-system-arm -s -S -M mps2-an385 -cpu cortex-m3 -monitor none -nographic -serial stdio -kernel ./output/RTOSDemo.out