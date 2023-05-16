cmake_minimum_required(VERSION 3.20.0)

set(OSLIB ${CMAKE_CURRENT_SOURCE_DIR}/../../oslib)
set(DTC_DEVICE_OVERLAY nrf52840dongle_nrf52840.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(base)

include_directories(app PRIVATE ${OSLIB})

set(BASE_DRIVERS ${OSLIB}/base_hci)
set(BASE_SOURCES ${BASE_DRIVERS}/base_hci.c ${BASE_DRIVERS}/base_hci.h)

target_sources(app PRIVATE src/main.c ${BASE_SOURCES})