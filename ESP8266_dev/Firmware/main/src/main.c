#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"

#include "freertos/task.h"
#include "freertos/event_groups.h"

/**
 * ADD: #define INCLUDE_eTaskGetState 1
 */
#include "freertos/FreeRTOSConfig.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_wifi_types.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "../include/wifi_config.h"

static void ICACHE_FLASH_ATTR wifi_connect_task(void *param);

TaskHandle_t wifi_scan_task_hndl;
TaskHandle_t wifi_connect_task_hndl;

static void ICACHE_FLASH_ATTR start_wifi_connect_task() {

    xTaskCreate(wifi_connect_task, "wifi_connect_task", 1024 * 8, NULL, 1, &wifi_connect_task_hndl);

}

static void ICACHE_FLASH_ATTR event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

        printf("Disconnected, retry to connect to the AP\n");

        if( eTaskGetState(wifi_connect_task_hndl) == eSuspended ) {
            ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
            printf("Resume WiFi connect Task\n");
            vTaskResume(wifi_connect_task_hndl);
        }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		printf("got ip:%s\n", ip4addr_ntoa(&event->ip_info.ip));

        ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));

        vTaskSuspend(wifi_connect_task_hndl);
    }
}

static void ICACHE_FLASH_ATTR wifi_connect_task(void *param) {

    wifi_config_t wifi_config;
    memcpy(wifi_config.sta.ssid, SSID, sizeof(SSID));
    memcpy(wifi_config.sta.password, PW, sizeof(PW));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    while(true) {
        const esp_err_t ret = esp_wifi_connect();
        printf("esp_wifi_connect returned '");
        switch(ret) {
            case ESP_ERR_WIFI_NOT_INIT: {
                printf("ESP_ERR_WIFI_NOT_INIT'\n");
            } break;
            case ESP_ERR_WIFI_NOT_STARTED: {
                printf("ESP_ERR_WIFI_NOT_START'\n");
            } break;
            case ESP_ERR_WIFI_CONN: {
                printf("ESP_ERR_WIFI_CONN'\n");
            } break;
            case ESP_ERR_WIFI_SSID: {
                printf("ESP_ERR_WIFI_SSID'\n");
            } break;
            case ESP_OK: {
                printf("ESP_OK'\n");
            } break;
            default: {
                printf("UNKNOWN RETURN TYPE!\n");
            }
        }

        vTaskDelay(7500 / portTICK_RATE_MS);
    }

}

static void ICACHE_FLASH_ATTR wifi_scan_task(void *param) {

    wifi_scan_config_t wifi_scan_config = {
        .scan_type = WIFI_SCAN_TYPE_PASSIVE,
        .scan_time = {
            .passive = 250
        }
    };

    uint16_t amount_of_records;
    wifi_ap_record_t records[25];

    ESP_ERROR_CHECK( esp_wifi_scan_start(&wifi_scan_config, true) );
    ESP_ERROR_CHECK( esp_wifi_scan_get_ap_records(&amount_of_records, records) );

    bool detected_config_ssid = false;

    for(uint16_t i = 0; i < amount_of_records; ++i) {

        const wifi_ap_record_t record = records[i];
        printf("RECORD %d:\n", i + 1);
        printf("\tSSID: %s\n", record.ssid);
        printf("\tCHANNEL: %d\n", record.primary);
        printf("\n");

        if( strcmp(SSID, (char*)record.ssid) == 0 ) {
            detected_config_ssid = true;
        }
    }

    if(detected_config_ssid) {
        start_wifi_connect_task();
    } else {
        printf("WIFI SCAN DIDN'T RETURN WIFI WITH SSID %s -> EXIT!\n", SSID);
    }

    vTaskSuspend(wifi_scan_task_hndl);
    vTaskDelete(wifi_scan_task_hndl);

}

void app_main() {

    setbuf(stdout, NULL);
    esp_wifi_set_auto_connect(false);

    ESP_ERROR_CHECK(nvs_flash_init());

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    xTaskCreate(wifi_scan_task, "wifi_scan_task", 1024 * 4, NULL, 1, wifi_scan_task_hndl);
}
