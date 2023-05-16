#ifndef BSU_HCI
#define BSU_HCI

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/usb/usb_device.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/types.h>

#define BSU_SHELL_THREAD_PRIORITY 1
#define BSU_SHELL_THREAD_STACK 512
#define BSU_PACKET_SIZE 32
#define BSU_RESPONSE_TIMEOUT 100

uint8_t ble_read_mobile(struct bt_conn *conn, uint8_t err, struct bt_gatt_read_params *params,
                        const void *data, uint16_t length);

/* Custom UUIDs For AHU and it's GATT Attributes */
#define UUID_BUFFER_SIZE 16
#define MOBILE_PACKET_SIZE 32

// Used to as a key to test against scanned UUIDs
uint16_t mobile_uuid_arr[] = {0xd1, 0x93, 0x68, 0x36, 0x79, 0x18, 0x22, 0x92,
                              0x27, 0x4A, 0x61, 0xEC, 0x07, 0xA8, 0xCB, 0xCD};

static struct bt_uuid_128 mobile_uuid = BT_UUID_INIT_128(
    0xd1, 0x93, 0x68, 0x36, 0x79, 0x18, 0x22, 0x92, 0x27, 0x4A, 0x61, 0xEC, 0x07, 0xA8, 0xCB, 0xCD);

// RX buffers for storing GATT characteristics upon read
uint8_t rx_mobile[MOBILE_PACKET_SIZE] = {0x00};

static struct bt_gatt_read_params read_params_mobile = {
    .func = ble_read_mobile,
    .handle_count = 0,
    .by_uuid.uuid = &mobile_uuid.uuid,
    .by_uuid.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE,
    .by_uuid.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE,
};

#endif