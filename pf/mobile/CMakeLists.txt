cmake_minimum_required(VERSION 3.20.0)

set(OSLIB ${CMAKE_CURRENT_SOURCE_DIR}/../../oslib)
set(DTC_DEVICE_OVERLAY nrf52840dongle_nrf52840.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bsu)

include_directories(app PRIVATE ${OSLIB})

set(MOBILE_DRIVERS ${OSLIB}/mobile_drivers)
set(MOBILE_SOURCES ${MOBILE_DRIVERS}/mobile_hci/mobile_hci.c ${MOBILE_DRIVERS}/mobile_hci/mobile_hci.h ${MOBILE_DRIVERS}/mobile_hci/mobile_bt.c ${MOBILE_DRIVERS}/mobile_hci/mobile_bt.h)

target_sources(app PRIVATE src/main.c ${MOBILE_SOURCES})
