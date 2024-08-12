/**
 * @file        mqtt.c
 * @author      Pranav (pranav@ethersys.ai)
 * @brief       MQTT
 * @version     0.1
 * @date        2024-05-24
 *
 * @copyright   Copyright (c) EtherSys 2024
 *
 */

#include "mqtt.h"
#include "net/mqtt_helper.h"

LOG_MODULE_REGISTER(MQTT, LOG_LEVEL_INF);

#define MQTT_MESSAGE_BUFFER_SIZE 2048

struct mqtt_client client;
struct sockaddr_storage broker;

/* Buffers for MQTT client. */
static uint8_t rx_buffer[MQTT_MESSAGE_BUFFER_SIZE];
static uint8_t tx_buffer[MQTT_MESSAGE_BUFFER_SIZE];
static uint8_t payload_buf[MQTT_MESSAGE_BUFFER_SIZE];

static void on_connect_cb(enum mqtt_conn_return_code return_code, bool session_present)
{
    LOG_INF("MQTT Return code: %d", return_code);

    int err = publish_mqtt_msg("Hello World!", "NORDIC/ETHER/TEST/CELLULAR");

    if (err != 0)
    {
        LOG_ERR("MQTT Publish failed");
    }
}

int publish_mqtt_msg(const char *message, const char *pub_topic)
{
    int err;

    struct mqtt_publish_param param = {
        .message.payload.data = (uint8_t *)message,
        .message.payload.len = strlen(message),
        .message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE,
        .message_id = k_uptime_get_32(),
        .message.topic.topic.utf8 = (uint8_t *)pub_topic,
        .message.topic.topic.size = strlen(pub_topic),
    };

    err = mqtt_helper_publish(&param);
    if (err)
    {
        LOG_WRN("Failed to send payload, err: %d", err);
        return -1;
    }
    LOG_INF("Published message: \"%.*s\" on topic: \"%.*s\"", param.message.payload.len,
            param.message.payload.data,
            param.message.topic.topic.size,
            param.message.topic.topic.utf8);
}

int mqtt_init()
{
    const uint8_t *device_id = client_id_get();

    struct mqtt_helper_conn_params broker_conn_params = {
        .hostname.ptr = MQTT_BROKER_HOST,
        .hostname.size = strlen(MQTT_BROKER_HOST),
        .user_name.ptr = MQTT_BROKER_USER,
        .user_name.size = strlen(MQTT_BROKER_USER),
        .password.ptr = MQTT_BROKER_PASS,
        .password.size = strlen(MQTT_BROKER_PASS),
        .device_id.ptr = device_id,
        .device_id.size = strlen(device_id),
    };

    struct mqtt_helper_cfg broker_cb_cfg = {
        .cb.on_connack = on_connect_cb,
    };

    int err = mqtt_helper_init(&broker_cb_cfg);
    if (err)
    {
        LOG_ERR("Failed to initialize MQTT client: %d", err);
        return 0;
    }
    LOG_INF("Connection to broker using mqtt_connect");
    err = mqtt_helper_connect(&broker_conn_params);
    while (err != 0)
    {
        LOG_ERR("Error in mqtt_connect: %d", err);
        k_sleep(K_SECONDS(2));
    }
    return err;
}

// int client_init(/* struct mqtt_client *client */)
// {
//     int err;
//     /* Initializes the client instance. */
//     mqtt_client_init(&client);

//     /* Resolves the configured hostname and initializes the MQTT broker structure */
//     err = broker_init();
//     if (err)
//     {
//         LOG_ERR("Failed to initialize broker connection");
//         return err;
//     }

//     /* MQTT client configuration */
//     client.broker = &broker;
//     client.evt_cb = mqtt_evt_handler;
//     client.client_id.utf8 = client_id_get();
//     client.client_id.size = strlen(client.client_id.utf8);
//     client.user_name->utf8 = (uint8_t *)MQTT_BROKER_USER;
//     client.user_name->size = sizeof(MQTT_BROKER_USER);
//     client.password->utf8 = (uint8_t *)MQTT_BROKER_PASS;
//     client.password->size = sizeof(MQTT_BROKER_PASS);
//     client.protocol_version = MQTT_VERSION_3_1_1;

//     client.rx_buf = rx_buffer;
//     client.rx_buf_size = sizeof(rx_buffer);
//     client.tx_buf = tx_buffer;
//     client.tx_buf_size = sizeof(tx_buffer);

//     /* We are not using TLS in Exercise 1 */
//     client.transport.type = MQTT_TRANSPORT_NON_SECURE;

//     return err;
// }

// void mqtt_evt_handler(struct mqtt_client *const c,
//                       const struct mqtt_evt *evt)
// {
//     int err;

//     switch (evt->type)
//     {
//     case MQTT_EVT_CONNACK:
//         if (evt->result != 0)
//         {
//             LOG_ERR("MQTT connect failed: %d", evt->result);
//             break;
//         }

//         LOG_INF("MQTT client connected");
//         subscribe(c, MQTT_SUBSCRIBE_TOPIC);
//         break;

//     case MQTT_EVT_DISCONNECT:
//         LOG_INF("MQTT client disconnected: %d", evt->result);
//         break;

//     case MQTT_EVT_PUBLISH:
//         /* Listen to published messages received from the broker and extract the message */
//         {
//             const struct mqtt_publish_param *p = &evt->param.publish;
//             LOG_INF("MQTT PUBLISH result=%d len=%d",
//                     evt->result, p->message.payload.len);

//             err = get_received_payload(c, p->message.payload.len);

//             if (p->message.topic.qos == MQTT_QOS_1_AT_LEAST_ONCE)
//             {
//                 const struct mqtt_puback_param ack = {
//                     .message_id = p->message_id};
//                 mqtt_publish_qos1_ack(c, &ack);
//             }

//             if (err >= 0)
//             {
//                 data_print("Received: ", payload_buf, p->message.payload.len);
//             }
//             else if (err == -EMSGSIZE)
//             {
//                 LOG_ERR("Received payload (%d bytes) is larger than the payload buffer size (%d bytes).",
//                         p->message.payload.len, sizeof(payload_buf));
//             }
//             else
//             {
//                 LOG_ERR("get_received_payload failed: %d", err);
//                 LOG_INF("Disconnecting MQTT client...");

//                 err = mqtt_disconnect(c);
//                 if (err)
//                 {
//                     LOG_ERR("Could not disconnect: %d", err);
//                 }
//             }
//         }
//         break;

//     case MQTT_EVT_PUBACK:
//         if (evt->result != 0)
//         {
//             LOG_ERR("MQTT PUBACK error: %d", evt->result);
//             break;
//         }

//         LOG_INF("PUBACK packet id: %u", evt->param.puback.message_id);
//         break;

//     case MQTT_EVT_SUBACK:
//         if (evt->result != 0)
//         {
//             LOG_ERR("MQTT SUBACK error: %d", evt->result);
//             break;
//         }

//         LOG_INF("SUBACK packet id: %u", evt->param.suback.message_id);
//         break;

//     case MQTT_EVT_PINGRESP:
//         if (evt->result != 0)
//         {
//             LOG_ERR("MQTT PINGRESP error: %d", evt->result);
//         }
//         break;

//     default:
//         LOG_INF("Unhandled MQTT event type: %d", evt->type);
//         break;
//     }
// }

// int broker_init(void)
// {
//     int err;
//     struct addrinfo *result;
//     struct addrinfo *addr;
//     struct addrinfo hints = {
//         .ai_family = AF_INET,
//         .ai_socktype = SOCK_STREAM};

//     err = getaddrinfo(MQTT_BROKER_HOST, NULL, &hints, &result);
//     if (err)
//     {
//         LOG_ERR("getaddrinfo failed: %d", err);
//         return -ECHILD;
//     }

//     addr = result;

//     /* Look for address of the broker. */
//     while (addr != NULL)
//     {
//         /* IPv4 Address. */
//         if (addr->ai_addrlen == sizeof(struct sockaddr_in))
//         {
//             struct sockaddr_in *broker4 =
//                 ((struct sockaddr_in *)&broker);
//             char ipv4_addr[NET_IPV4_ADDR_LEN];

//             broker4->sin_addr.s_addr =
//                 ((struct sockaddr_in *)addr->ai_addr)
//                     ->sin_addr.s_addr;
//             broker4->sin_family = AF_INET;
//             broker4->sin_port = htons(MQTT_BROKER_PORT);

//             inet_ntop(AF_INET, &broker4->sin_addr.s_addr,
//                       ipv4_addr, sizeof(ipv4_addr));
//             LOG_INF("IPv4 Address found %s", (char *)(ipv4_addr));

//             break;
//         }
//         else
//         {
//             LOG_ERR("ai_addrlen = %u should be %u or %u",
//                     (unsigned int)addr->ai_addrlen,
//                     (unsigned int)sizeof(struct sockaddr_in),
//                     (unsigned int)sizeof(struct sockaddr_in6));
//         }

//         addr = addr->ai_next;
//     }

//     /* Free the address. */
//     freeaddrinfo(result);

//     return err;
// }

const uint8_t *client_id_get(void)
{
    static uint8_t client_id[CLIENT_ID_LEN];

    char imei_buf[CGSN_RESPONSE_LENGTH + 1];
    int err;

    err = nrf_modem_at_cmd(imei_buf, sizeof(imei_buf), "AT+CGSN");
    if (err)
    {
        LOG_ERR("Failed to obtain IMEI, error: %d", err);
        goto exit;
    }

    imei_buf[IMEI_LEN] = '\0';

    snprintf(client_id, sizeof(client_id), "nrf-%.*s", IMEI_LEN, imei_buf);

exit:
    LOG_DBG("client_id = %s", (char *)(client_id));

    return client_id;
}

// int get_received_payload(struct mqtt_client *c, size_t length)
// {
//     int ret;
//     int err = 0;

//     /* Return an error if the payload is larger than the payload buffer.
//      * Note: To allow new messages, we have to read the payload before returning.
//      */
//     if (length > sizeof(payload_buf))
//     {
//         err = -EMSGSIZE;
//     }

//     /* Truncate payload until it fits in the payload buffer. */
//     while (length > sizeof(payload_buf))
//     {
//         ret = mqtt_read_publish_payload_blocking(
//             c, payload_buf, (length - sizeof(payload_buf)));
//         if (ret == 0)
//         {
//             return -EIO;
//         }
//         else if (ret < 0)
//         {
//             return ret;
//         }

//         length -= ret;
//     }

//     ret = mqtt_readall_publish_payload(c, payload_buf, length);
//     if (ret)
//     {
//         return ret;
//     }

//     return err;
// }

// int data_publish(/* struct mqtt_client *c,  */ enum mqtt_qos qos,
//                  uint8_t *data, size_t len, const char *topic)
// {
//     struct mqtt_publish_param param;

//     param.message.topic.qos = qos;
//     param.message.topic.topic.utf8 = topic;
//     param.message.topic.topic.size = strlen(topic);
//     param.message.payload.data = data;
//     param.message.payload.len = len;
//     param.message_id = sys_rand32_get();
//     param.dup_flag = 0;
//     param.retain_flag = 0;

//     data_print("Publishing: ", data, len);
//     LOG_INF("to topic: %s len: %u", topic, (unsigned int)strlen(topic));

//     return mqtt_publish(&client, &param);
// }

// int subscribe(struct mqtt_client *const c, const char *topic)
// {
//     struct mqtt_topic subscribe_topic = {
//         .topic = {
//             .utf8 = topic,
//             .size = strlen(topic)},
//         .qos = MQTT_QOS_1_AT_LEAST_ONCE};

//     const struct mqtt_subscription_list subscription_list = {
//         .list = &subscribe_topic,
//         .list_count = 1,
//         .message_id = 1234};

//     LOG_INF("Subscribing to: %s len %u", topic,
//             (unsigned int)strlen(topic));

//     return mqtt_subscribe(c, &subscription_list);
// }

// void data_print(uint8_t *prefix, uint8_t *data, size_t len)
// {
//     char buf[len + 1];

//     memcpy(buf, data, len);
//     buf[len] = 0;
//     LOG_INF("%s%s", (char *)prefix, (char *)buf);
// }