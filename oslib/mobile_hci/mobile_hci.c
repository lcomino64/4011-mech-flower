#include "mobile_hci.h"

struct bt_conn *default_conn;

/* Register log module */
LOG_MODULE_REGISTER(mobile, CONFIG_LOG_DEFAULT_LEVEL);

/* Queue to store found devices */
K_MSGQ_DEFINE(bt_rx_msgq, MOBILE_PACKET_SIZE, MOBILE_QUEUE_SIZE, 4);

/* UUIDs */
static struct bt_uuid_128 mobile_uuid = BT_UUID_INIT_128(
    0xd1, 0x93, 0x68, 0x36, 0x79, 0x18, 0x22, 0x92, 0x27, 0x4A, 0x61, 0xEC, 0x07, 0xA8, 0xCB, 0xCD);

/* Advertising Data */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0xd1, 0x93, 0x68, 0x36, 0x79, 0x18, 0x22, 0x92, 0x27, 0x4A,
                  0x61, 0xEC, 0x07, 0xA8, 0xCB, 0xCD),
};

static uint8_t node_mobile[MOBILE_PACKET_SIZE];

/**
 * @brief Callback function for read request by bsu
 */
static ssize_t read_mobile(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                           uint16_t len, uint16_t offset) {
    char response[MOBILE_PACKET_SIZE];

    // wait until response
    k_msgq_get(&bt_rx_msgq, &response, K_NO_WAIT);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, response, strlen(response));
}

/* Define the gatt characteristics */
BT_GATT_SERVICE_DEFINE(mobile_bt, BT_GATT_PRIMARY_SERVICE(&mobile_uuid),
                       BT_GATT_CHARACTERISTIC(&mobile_uuid.uuid, BT_GATT_CHRC_READ,
                                              BT_GATT_PERM_READ, read_mobile, NULL, &node_mobile));

/* Callback function for connection */
static void connected(struct bt_conn *connected, uint8_t err) {
    if (err) {
        LOG_ERR("Connection failed (err 0x%02x)", err);
    } else {
        default_conn = bt_conn_ref(connected);
        LOG_INF("Connected");
    }
}

/* Callback function for disconnection */
static void disconnected(struct bt_conn *connected, uint8_t reason) {
    bt_conn_unref(connected);
    LOG_ERR("Disconnected (reason 0x%02x)", reason);
}

BT_CONN_CB_DEFINE(bt_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

/**
 * @brief Main thread for Mobile Unit
 */
void mobile_thread(void) {
    if (bt_enable(NULL)) {
        LOG_ERR("Bluetooth init failed");
        return;
    }

    int err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return;
    }

    while (1) {
        k_msleep(100);
    }
}

K_THREAD_DEFINE(mobile_thread_tid, MOBILE_THREAD_STACK, mobile_thread, NULL, NULL, NULL,
                MOBILE_THREAD_PRIORITY, 0, 0);