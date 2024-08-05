/**
 * @file        mqtt.h
 * @author      Pranav (pranav@ethersys.ai)
 * @brief       MQTT
 * @version     0.1
 * @date        2024-05-22
 *
 * @copyright Copyright (c) EtherSys 2024
 *
 */

#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/socket.h>
#include "zephyr/logging/log.h"
#include <nrf_modem_at.h>
#include <zephyr/random/random.h>

#define IMEI_LEN 15
#define CGSN_RESPONSE_LENGTH (IMEI_LEN + 6 + 1) /* Add 6 for \r\nOK\r\n and 1 for \0 */
#define CLIENT_ID_LEN sizeof("nrf-") + IMEI_LEN

#define MQTT_BROKER_HOST ""
#define MQTT_BROKER_PORT 0
#define MQTT_BROKER_USER ""
#define MQTT_BROKER_PASS "!"
#define MQTT_SUBSCRIBE_TOPIC "DEV/NORDIC/NRF9160"

int mqtt_init();
// int mqtt_keep_connection();
// int mqtt_publish();
// int mqtt_subscribe();
// int mqtt_deinit();

int client_init(/* struct mqtt_client *client */);
void mqtt_evt_handler(struct mqtt_client *const c,
                      const struct mqtt_evt *evt);
const uint8_t *client_id_get(void);
int broker_init(void);
int get_received_payload(struct mqtt_client *c, size_t length);
int data_publish(/* struct mqtt_client *c,  */enum mqtt_qos qos,
				 uint8_t *data, size_t len, const char *topic);
int subscribe(struct mqtt_client *const c, const char *topic);
void data_print(uint8_t *prefix, uint8_t *data, size_t len);

#endif