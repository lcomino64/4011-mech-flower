#!/bin/bash

# Check for argument
if [ -z "$1" ]; then
  echo "Error: no directory provided."
  exit 1
fi

# Kill any screens
pkill SCREEN

# Build and flash the specified directory
west build -p always -b nrf52840dongle_nrf52840 $1 || exit 1

# Package the application for the bootloader, using nrfutil
nrfutil pkg generate --hw-version 52 --sd-req=0x00 \
        --application build/zephyr/zephyr.hex \
        --application-version 1 temp.zip

# Flash the board
nrfutil dfu usb-serial -pkg temp.zip -p /dev/cu.usbmodemC7C8FE462DC91 

