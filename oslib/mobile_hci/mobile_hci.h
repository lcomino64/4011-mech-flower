#ifndef MOBILE_HCI
#define MOBILE_HCI

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/zephyr.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/hrs.h>
#include <zephyr/bluetooth/services/ias.h>
#include <zephyr/bluetooth/uuid.h>

#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/ccs811.h>

#define MOBILE_PACKET_SIZE 32
#define MOBILE_QUEUE_SIZE 8
#define MOBILE_THREAD_PRIORITY 1
#define MOBILE_THREAD_STACK 1024

#define NODE_COUNT 8

#endif