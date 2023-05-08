#!/bin/bash

# Kill any running monitors
pkill -f "west espressif monitor" 

# Build and flash the specified directory
west build -p always -b esp32c3_devkitm pf/bsu || exit 1
west flash || exit 1
