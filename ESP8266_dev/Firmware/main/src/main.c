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

#include "driver/i2c.h"

#include "../include/wifi_config.h"

#define I2C_MASTER_SDA_GPIO     4
#define I2C_MASTER_SCK_GPIO     5

#define BMP280_I2C_WRITE_ADDR   (0b11101110)
#define BMP280_I2C_READ_ADDR    (0b11101111)

#define BMP280_REG_ADDR_ID      (0xD0)

TaskHandle_t wifi_scan_task_hndl = NULL;
TaskHandle_t wifi_connect_task_hndl = NULL;
TaskHandle_t i2c_bmp280_task_hndl = NULL;

static void ICACHE_FLASH_ATTR i2c_bmp280_task(void *arg) {

    const bool slave_ack_en = true;

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_GPIO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCK_GPIO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.

    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));

    while(true) {

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, BMP280_I2C_WRITE_ADDR, slave_ack_en);
        i2c_master_write_byte(cmd, BMP280_REG_ADDR_ID, slave_ack_en);

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, BMP280_I2C_READ_ADDR, slave_ack_en);

        uint8_t bmp280_id = 0x00;
        i2c_master_read(cmd, &bmp280_id, 1, I2C_MASTER_LAST_NACK);

        const esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
        if(ret != ESP_OK) {
            printf("FAILED I2C!\n");
        }

        i2c_cmd_link_delete(cmd);

        printf("BMP 280 ID: %d\n", bmp280_id);

        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}

static void ICACHE_FLASH_ATTR event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

        printf("Disconnected, retry to connect to the AP '%s'\n", SSID);

        if( eTaskGetState(wifi_connect_task_hndl) == eSuspended ) {

            ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

            printf("Resume WiFi connect task\n");
            vTaskResume(wifi_connect_task_hndl);
        }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		printf("Got ip:%s\n", ip4addr_ntoa(&event->ip_info.ip));

        ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));

        vTaskSuspend(wifi_connect_task_hndl);

        if( i2c_bmp280_task_hndl == NULL ) {
            xTaskCreate(i2c_bmp280_task, "i2c_bmp280_task", 1024 * 8, NULL, 1, &i2c_bmp280_task_hndl);
        }
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
    
    while(true) {

        bool detected_config_ssid = false;

        ESP_ERROR_CHECK( esp_wifi_scan_start(&wifi_scan_config, true) );
        ESP_ERROR_CHECK( esp_wifi_scan_get_ap_records(&amount_of_records, records) );

        printf("WIFI SCAN RESULTS:\n");

        for(uint16_t i = 0; i < amount_of_records; ++i) {

            const wifi_ap_record_t record = records[i];
            printf("%d: SSID: %s\n", i + 1, record.ssid);
            printf("\tCHANNEL: %d\n", record.primary);
            printf("\n");

            if( strcmp(SSID, (char*)record.ssid) == 0 ) {
                detected_config_ssid = true;
            }
        }

        if(detected_config_ssid) {
            printf("FOUND WIFI WITH SSID  '%s'!\n", SSID);

            if(wifi_connect_task_hndl == NULL) {
                xTaskCreate(wifi_connect_task, "wifi_connect_task", 1024 * 8, NULL, 1, &wifi_connect_task_hndl);
            } else {
                vTaskResume(wifi_connect_task_hndl);
            }

            vTaskSuspend(wifi_scan_task_hndl);
        }

        vTaskDelay(7500 / portTICK_RATE_MS);
    }
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

    xTaskCreate(wifi_scan_task, "wifi_scan_task", 1024 * 4, NULL, 1, &wifi_scan_task_hndl);
}
