#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "lte/lte.h"
#include "mqtt/mqtt.h"

LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_INF);

int main(void)
{
    int err = nrf_modem_lib_init();
    if (err)
    {
        LOG_ERR("Modem library initialization failed, error: %d", err);
        return err;
    }

    err = modem_configure();
    if (err)
    {
        LOG_ERR("Modem Configure: %d", err);
        return err;
    }

    err = broker_init();
    if (err)
    {
        LOG_ERR("Broker Connection Fail: %d", err);
        return err;
    }

    LOG_INF("Delay of 30s to make sure nrf9160 is connected to the network");
    k_msleep(30 * 1000);

    err = mqtt_init();
    if (err)
    {
        LOG_ERR("MQTT Connection Fail: %d", err);
        return err;
    }

    uint8_t *data = "Hello World!";
    err = data_publish(0, data, sizeof(data), "ETHER/NORDIC/NRF9160");
    if (err)
    {
        LOG_ERR("MQTT Message Send Fail: %d", err);
        return err;
    }

    return 0;
}
