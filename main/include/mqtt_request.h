#include <stdio.h>
#include "mqtt_client.h"

#ifndef __MQTT_REQUEST__
#define __MQTT_REQUEST__

esp_mqtt_client_handle_t mqtt_app_start(void);
int32_t get_time_ms();
int32_t get_req_num();

#endif /* __MQTT_REQUEST__ */
