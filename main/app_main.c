/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
//#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
//#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/spi_master.h"
#include "esp_config.h"
#include "wifi_connect.h"
#include "mqtt_request.h"
#include "spi_sensor.h"

/* Definitions located in "esp_config.h"
  #define URI_MQTT 	""
  #define TOPIC_PUB_TEMP	""		// Temperature Data
  #define TOPIC_SUB_TIME	""		// Sampling time
  #define TOPIC_SUB_NREQ	""
*/

static const char *TAG = "MQTT_MAIN";
static char sendTemp[20];
static esp_mqtt_client_handle_t client = NULL;
/* Sensor variables task */
static int16_t temperature = 0;
static uint16_t timePeriodo = 1000;

/* Task: SPI MAX6675 - Temperature Sensor Thermocouple K  */
void temp_task(void * pvParameters) {
  spi_device_handle_t spi = (spi_device_handle_t) pvParameters;
  while(true)
  {
    temperature = read_temp(spi);
    printf("SPI INT = %d temp=%f\n", temperature, temperature * 0.25);
    vTaskDelay(timePeriodo / portTICK_PERIOD_MS);
    //counter_t++;
  }
  vTaskDelete(NULL);
}

/* Task: MQTT Publish - Value Temperature Sensor */
void mqtt_send_task(void *pvParameter){
	int counter = 0;
	while(counter<10){
		counter++;
		sprintf(sendTemp,"%d",temperature);
		ESP_LOGI(TAG, "Contador: %d bytes", counter);
        vTaskDelay(get_time_ms() / portTICK_PERIOD_MS);	
		
        int msg_id = esp_mqtt_client_publish(client, TOPIC_PUB_TEMP, sendTemp, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
	}
	vTaskDelete(NULL);
}

void app_main(void)
{
    spi_device_handle_t spi;
    spi = spi_init();

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
/*
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
*/

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    /* 
     * This helper function configures Wi-Fi. Read "Establishing Wi-Fi" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    client = mqtt_app_start();

    //esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    vTaskDelay(2000/portTICK_PERIOD_MS);

    xTaskCreate(&temp_task, "temperature_task", 4096, spi, 5, NULL);
    xTaskCreate( &mqtt_send_task, "counter_task", 2048, client, 5, NULL );
}
