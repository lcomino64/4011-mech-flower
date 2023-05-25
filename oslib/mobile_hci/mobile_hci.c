#include "mobile_hci.h"

struct bt_conn *default_conn;
const struct device *hts221_dev;
const struct device *ccs811_dev;

/* Register log module */
LOG_MODULE_REGISTER(mobile, CONFIG_LOG_DEFAULT_LEVEL);

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

static bool app_fw_2;

static const char *now_str(void) {
    static char buf[16]; /* ...HH:MM:SS.MMM */
    uint32_t now = k_uptime_get_32();
    unsigned int ms = now % MSEC_PER_SEC;
    unsigned int s;
    unsigned int min;
    unsigned int h;

    now /= MSEC_PER_SEC;
    s = now % 60U;
    now /= 60U;
    min = now % 60U;
    now /= 60U;
    h = now;

    snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u", h, min, s, ms);
    return buf;
}

static void do_main(const struct device *dev) {
    int rc = do_fetch(dev);

    if (rc == 0) {
        printk("Timed fetch got %d\n", rc);
    } else if (-EAGAIN == rc) {
        printk("Timed fetch got stale data\n");
    } else {
        printk("Timed fetch failed: %d\n", rc);
    }
}

static void process_sample(const struct device *dev) {
    static unsigned int obs;
    struct sensor_value temp, hum;
    if (sensor_sample_fetch(dev) < 0) {
        printf("Sensor sample update error\n");
        return;
    }

    if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
        printf("Cannot read HTS221 temperature channel\n");
        return;
    }

    if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
        printf("Cannot read HTS221 humidity channel\n");
        return;
    }

    ++obs;
    printf("Observation:%u\n", obs);

    /* display temperature */
    printf("Temperature:%.1f C\n", sensor_value_to_double(&temp));

    /* display humidity */
    printf("Relative Humidity:%.1f%%\n", sensor_value_to_double(&hum));
}

static void hts221_handler(const struct device *dev, const struct sensor_trigger *trig) {
    process_sample(dev);
}

/**
 * @brief Callback function for read request by bsu
 */
static ssize_t read_mobile(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                           uint16_t len, uint16_t offset) {
    char response[MOBILE_PACKET_SIZE];

    struct sensor_value co2, tvoc, voltage, current;
    int rc = 0;
    int baseline = -1;

    if (rc == 0) {
        rc = sensor_sample_fetch(ccs811_dev);
    }
    if (rc == 0) {
        const struct ccs811_result_type *rp = ccs811_result(ccs811_dev);
        sensor_channel_get(ccs811_dev, SENSOR_CHAN_CO2, &co2);
        sensor_channel_get(ccs811_dev, SENSOR_CHAN_VOC, &tvoc);
        sensor_channel_get(ccs811_dev, SENSOR_CHAN_VOLTAGE, &voltage);
        sensor_channel_get(ccs811_dev, SENSOR_CHAN_CURRENT, &current);

        if (app_fw_2 && !(rp->status & CCS811_STATUS_DATA_READY)) {
            printk("STALE DATA\n");
        }

        if (rp->status & CCS811_STATUS_ERROR) {
            printk("ERROR: %02x\n", rp->error);
        }
    }

    static unsigned int obs;
    struct sensor_value temp, hum;
    if (sensor_sample_fetch(hts221_dev) < 0) {
        printf("Sensor sample update error\n");
        return;
    }

    if (sensor_channel_get(hts221_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
        printf("Cannot read HTS221 temperature channel\n");
        return;
    }

    if (sensor_channel_get(hts221_dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
        printf("Cannot read HTS221 humidity channel\n");
        return;
    }

    // /* display temperature */
    // printf("Temperature:%.1f C\n", sensor_value_to_double(&temp));

    // /* display humidity */
    // printf("Relative Humidity:%.1f%%\n", sensor_value_to_double(&hum));
    sprintf(response, "%.1f,%.1f,%u,%u", sensor_value_to_double(&temp),
            sensor_value_to_double(&hum), co2.val1, tvoc.val1);
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

    ccs811_dev = device_get_binding(DT_LABEL(DT_INST(0, ams_ccs811)));
    struct ccs811_configver_type cfgver;
    int rc;

    if (!ccs811_dev) {
        LOG_ERR("Failed to get device binding");
        return;
    }

    LOG_INF("device is %p, name is %s\n", ccs811_dev, ccs811_dev->name);

    rc = ccs811_configver_fetch(ccs811_dev, &cfgver);
    if (rc == 0) {
        LOG_INF("HW %02x; FW Boot %04x App %04x ; mode %02x\n", cfgver.hw_version,
                cfgver.fw_boot_version, cfgver.fw_app_version, cfgver.mode);
        app_fw_2 = (cfgver.fw_app_version >> 8) > 0x11;
    }

    hts221_dev = device_get_binding("HTS221");

    if (hts221_dev == NULL) {
        printf("Could not get HTS221 device\n");
        return;
    }

    if (IS_ENABLED(CONFIG_HTS221_TRIGGER)) {
        struct sensor_trigger trig = {
            .type = SENSOR_TRIG_DATA_READY,
            .chan = SENSOR_CHAN_ALL,
        };
        if (sensor_trigger_set(hts221_dev, &trig, hts221_handler) < 0) {
            printf("Cannot configure trigger\n");
            return;
        }
    }

    // while (!IS_ENABLED(CONFIG_HTS221_TRIGGER)) {
    //     process_sample(dev);
    //     k_sleep(K_MSEC(2000));
    // }

    while (1) {
        k_msleep(100);
    }
}

K_THREAD_DEFINE(mobile_thread_tid, MOBILE_THREAD_STACK, mobile_thread, NULL, NULL, NULL,
                MOBILE_THREAD_PRIORITY, 0, 0);