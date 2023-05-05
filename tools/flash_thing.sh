#!/bin/bash

# Check for argument
if [ -z "$1" ]; then
  echo "Error: no directory provided."
  exit 1
fi

# Build and flash the specified directory
west build -p always -b thingy52_nrf52832 $1 || exit 1
west flash || exit 1
