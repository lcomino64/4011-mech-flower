#!/bin/bash

# Build and flash the specified directory
west build -p always -b thingy52_nrf52832 pf/mobile || exit 1
west flash || exit 1
