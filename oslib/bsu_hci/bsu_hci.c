#include "bsu_hci.h"

LOG_MODULE_REGISTER(base, CONFIG_LOG_DEFAULT_LEVEL);

// Declare comms to use cdc_acm_uart1
#define COMMS_DEVICE_NODE DT_CHOSEN(zephyr_usb_comms)
const struct device *comms_dev = DEVICE_DT_GET(COMMS_DEVICE_NODE);

static struct bt_conn *default_conn;

// Globals
long gatt_read_sample_rate = 2;         // 2s minimum
uint8_t gatt_read_sampling_enabled = 0; // false

/**
 * @brief helper method to convert a string to integer
 */
long convert_str_to_int(char *str) {
    char *endptr;
    long num;

    num = strtol(str, &endptr, 10);

    if (endptr == str) {
        LOG_ERR("No digits were found");
    } else if (*endptr != '\0') {
        LOG_ERR("Invalid input: %s", endptr);
    }
    return num;
}

/**
 * @brief Helper method to convert received GATT values (hex) to string
 */
void convert_hex_buf_to_str_mobile(uint8_t *rx_buf, uint16_t length) {
    char tx_buf[100];
    char str[length + 1];

    for (uint8_t i = 0; i < length; i++) {
        str[i] = (char)rx_buf[i];
    }
    str[length] = '\0';

    sprintf(tx_buf, "%s\r\n", str);

    printk("%s", tx_buf);
}

/**
 * @brief Read Mobile
 */
uint8_t ble_read_mobile(struct bt_conn *conn, uint8_t err, struct bt_gatt_read_params *params,
                        const void *data, uint16_t length) {
    memcpy(&rx_mobile, data, length);
    convert_hex_buf_to_str_mobile(rx_mobile, length);
    memset(rx_mobile, 0, MOBILE_PACKET_SIZE);
    return 0;
}

/**
 * @brief Once a device is found, parse the advertised information
 */
static bool parse_found_device_mobile(struct bt_data *data, void *user_data) {
    bt_addr_le_t *addr = user_data;
    int i;
    int matchedCount = 0;

    if (data->type == BT_DATA_UUID128_ALL) {
        uint16_t temp = 0;
        for (i = 0; i < data->data_len; i++) {
            temp = data->data[i];
            if (temp == mobile_uuid_arr[i]) {
                matchedCount++;
            }
        }

        if (matchedCount == UUID_BUFFER_SIZE) {
            int err = bt_le_scan_stop();
            k_msleep(10);

            if (err) {
                LOG_ERR("Stop LE scan failed (err %d)", err);
                return true;
            }

            struct bt_le_conn_param *param = BT_LE_CONN_PARAM_DEFAULT;

            err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, param, &default_conn);
            if (err) {
                LOG_ERR("Create conn failed (err %d)", err);
            }
            return false;
        }
    }
    return true;
}

/**
 * @brief Callback for when a device is found
 */
static void device_found_mobile(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                                struct net_buf_simple *ad) {
    char addr_str[BT_ADDR_LE_STR_LEN];

    if (default_conn) {
        LOG_ERR("Already connected to Mobile");
        if (bt_le_scan_stop()) {
            LOG_INF("Stopped scanning");
            return;
        }
    }

    /* We're only interested in connectable events */
    if (type == BT_GAP_ADV_TYPE_ADV_IND || type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
        bt_data_parse(ad, parse_found_device_mobile, (void *)addr);
    }

    if (!default_conn) {
        return;
    } else {
        bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
        LOG_INF("Mobile found as: %s (RSSI %d)", addr_str, rssi);
    }

    if (bt_le_scan_stop()) {
        LOG_INF("Stopped scanning");
        return;
    }
}

/**
 * @brief Start scanning for devices
 */
void start_scan_mobile() {
    int err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, device_found_mobile);
    if (err) {
        LOG_ERR("Scanning failed to start (err %d)", err);
        return;
    }
    LOG_INF("Scanning started");
}

/**
 * @brief Callback for when device is connected
 */
static void connected(struct bt_conn *conn, uint8_t err) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err) {
        LOG_INF("Failed to connect to %s (%u)", addr, err);

        bt_conn_unref(default_conn);
        default_conn = NULL;

        start_scan_mobile();
        return;
    }

    if (conn != default_conn) {
        return;
    }
    LOG_INF("Connected: %s", addr);
}

/**
 * @brief Callback for when device disconnects
 */
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    char addr[BT_ADDR_LE_STR_LEN];

    if (conn != default_conn) {
        return;
    }

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_ERR("Disconnected: %s (reason 0x%02x)", addr, reason);

    bt_conn_unref(default_conn);
    default_conn = NULL;
    start_scan_mobile();
}

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

/**
 * @brief Thread for continuous sampling of mobile
 */
void bsu_gatt_read_thread_mobile(void) {
    while (1) {
        if (default_conn && gatt_read_sampling_enabled) {
            bt_gatt_read(default_conn, &read_params_mobile);
        }
        k_msleep(gatt_read_sample_rate * 1000);
    }
}

/**
 * @brief Main thread for Base Station Unit (BSU) command line interface implementation.
 */
void bsu_shell_thread(void) {
    if (!device_is_ready(comms_dev)) {
        LOG_ERR("Device not ready");
        return;
    }

    if (usb_enable(NULL)) {
        LOG_ERR("Failed to enable USB");
        return;
    }

    if (bt_enable(NULL)) {
        LOG_ERR("Bluetooth init failed");
        return;
    }
    bt_conn_cb_register(&conn_callbacks);

    uint32_t dtr = 0;
    while (!dtr) {
        uart_line_ctrl_get(comms_dev, UART_LINE_CTRL_DTR, &dtr);
        k_sleep(K_MSEC(100));
    }

    while (1) {
        k_msleep(100);
    }
}

K_THREAD_DEFINE(bsu_gatt_read_thread_mobile_tid, BSU_SHELL_THREAD_STACK,
                bsu_gatt_read_thread_mobile, NULL, NULL, NULL, BSU_SHELL_THREAD_PRIORITY + 2, 0, 0);