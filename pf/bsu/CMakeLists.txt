cmake_minimum_required(VERSION 3.20.0)

set(OSLIB ${CMAKE_CURRENT_SOURCE_DIR}/../../oslib)
set(DTC_DEVICE_OVERLAY nrf52840dongle_nrf52840.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bsu)

include_directories(app PRIVATE ${OSLIB})

set(BSU_DRIVERS ${OSLIB}/bsu_hci)
set(BSU_SOURCES ${BSU_DRIVERS}/bsu_hci.c ${BSU_DRIVERS}/bsu_hci.h)

target_sources(app PRIVATE src/main.c ${BSU_SOURCES})