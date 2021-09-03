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

#define I2C_MASTER_SDA_GPIO                     4
#define I2C_MASTER_SCK_GPIO                     5

#define BMP280_I2C_WRITE_ADDR                   (0b11101110)
#define BMP280_I2C_READ_ADDR                    (0b11101111)

#define BMP280_REG_ADDR_ID                      (0xD0)

#define BMP280_REG_ADDR_STATUS                  (0xF3)
#define BMP280_STATUS_MEASURING                 (1 << 3)

#define BMP280_REG_ADDR_CTRL_MEAS               (0xF4)
#define BMP280_REG_CTRL_MEAS_TEMP_OVERSAMPLING  (0xE0)
#define BMP280_REG_CTRL_MEAS_PRESSURE_SKIP      (0x00)
#define BMP280_REG_CTRL_MEAS_NORMAL_MODE        (0x03)

#define BMP280_REG_ADDR_CONFIG                  (0xF5)
#define BMP280_REG_CONFIG_STANDBY_2000MS        (0xC0)

#define BMP280_REG_ADDR_TRIMMING_VALUES_START   (0x88)

#define BMP280_REG_ADDR_TEMP_MSB                (0xFA)
#define BMP280_REG_ADDR_TEMP_LSB                (0xFB)
#define BMP280_REG_ADDR_TEMP_XLSB               (0xFC)

#define BMP280_SLAVE_ACK_EN         (true)

TaskHandle_t wifi_scan_task_hndl = NULL;
TaskHandle_t wifi_connect_task_hndl = NULL;
TaskHandle_t i2c_bmp280_task_hndl = NULL;

/**
 * Code from https://cdn-shop.adafruit.com/datasheets/BST-BMP280-DS001-11.pdf
 */
static int32_t ICACHE_FLASH_ATTR calculate_temp_from_raw_value(int16_t dig_T1, uint16_t dig_T2, uint16_t dig_T3, uint32_t value) {

    int32_t part1 = (value >> 3) - (dig_T1 << 1);
    int32_t var1  = (part1 * dig_T2) >> 11;

    int32_t part2 = (value >> 4) - dig_T1;
    int32_t var2  = (((part2 * part2) >> 12) * dig_T3 ) >> 14;

    int32_t t_fine = var1 + var2;
    
    return (t_fine * 5 + 128) >> 8;
}


static void ICACHE_FLASH_ATTR i2c_bmp280_read_reg(uint8_t reg_start_addr, uint8_t read_length, uint8_t *ret_param) {

    i2c_cmd_handle_t read_cmd = i2c_cmd_link_create();
    i2c_master_start(read_cmd);
    i2c_master_write_byte(read_cmd, BMP280_I2C_WRITE_ADDR, BMP280_SLAVE_ACK_EN);
    i2c_master_write_byte(read_cmd, reg_start_addr, BMP280_SLAVE_ACK_EN);

    i2c_master_start(read_cmd);
    i2c_master_write_byte(read_cmd, BMP280_I2C_READ_ADDR, BMP280_SLAVE_ACK_EN);

    for(uint8_t i = 0; i < read_length; ++i) {

        if( i == read_length - 1 ) {
            i2c_master_read(read_cmd, &ret_param[i], 1, I2C_MASTER_NACK);
        } else {
            i2c_master_read(read_cmd, &ret_param[i], 1, I2C_MASTER_ACK);
        }
    }

    const esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, read_cmd, 1000 / portTICK_RATE_MS);
    if(ret != ESP_OK) {
        printf("FAILED I2C BMP280 READ CMD [%d]\n", reg_start_addr);
    }

    i2c_cmd_link_delete(read_cmd);
}

static void ICACHE_FLASH_ATTR i2c_bmp280_write_reg(uint8_t reg_addr, uint8_t reg_data) {

    i2c_cmd_handle_t write_cmd = i2c_cmd_link_create();
    i2c_master_start(write_cmd);
    i2c_master_write_byte(write_cmd, BMP280_I2C_WRITE_ADDR, BMP280_SLAVE_ACK_EN);
    i2c_master_write_byte(write_cmd, reg_addr, BMP280_SLAVE_ACK_EN);
    i2c_master_write_byte(write_cmd, reg_data, BMP280_SLAVE_ACK_EN);
    i2c_master_stop(write_cmd);

    const esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, write_cmd, 1000 / portTICK_RATE_MS);
    if(ret != ESP_OK) {
        printf("FAILED I2C BMP280 WRITE CMD [%d -> %d]\n", reg_addr, reg_data);
    }

    i2c_cmd_link_delete(write_cmd);
}

static void ICACHE_FLASH_ATTR i2c_bmp280_task(void *arg) {

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_GPIO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCK_GPIO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.

    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));

    uint8_t bmp280_id;
    i2c_bmp280_read_reg(BMP280_REG_ADDR_ID, 1, &bmp280_id);

    printf("BMP 280 ID: %d\n", bmp280_id);

    i2c_bmp280_write_reg(BMP280_REG_ADDR_CTRL_MEAS, BMP280_REG_CTRL_MEAS_TEMP_OVERSAMPLING | BMP280_REG_CTRL_MEAS_PRESSURE_SKIP | BMP280_REG_CTRL_MEAS_NORMAL_MODE);
    i2c_bmp280_write_reg(BMP280_REG_ADDR_CONFIG, BMP280_REG_CONFIG_STANDBY_2000MS);

    uint8_t dig_Ts[6];
    i2c_bmp280_read_reg(BMP280_REG_ADDR_TRIMMING_VALUES_START, 6, dig_Ts);

    const uint8_t dig_T1_lsb = dig_Ts[0];
    const uint8_t dig_T1_msb = dig_Ts[1];
    const uint16_t dig_T1 = (dig_T1_msb << 8) | dig_T1_lsb;

    const uint8_t dig_T2_lsb = dig_Ts[2];
    const uint8_t dig_T2_msb = dig_Ts[3];
    const int16_t dig_T2 = (int16_t)( (dig_T2_msb << 8) | dig_T2_lsb );

    const uint8_t dig_T3_lsb = dig_Ts[4];
    const uint8_t dig_T3_msb = dig_Ts[5];
    const int16_t dig_T3 = (int16_t)( (dig_T3_msb << 8) | dig_T3_lsb );

    printf("dig_T1: %hu\ndig_T2:%hi\ndig_T3: %hi\n", dig_T1, dig_T2, dig_T3);

    while(true) {

        uint8_t t_raw[3];
        i2c_bmp280_read_reg(BMP280_REG_ADDR_TEMP_MSB, 3, t_raw);

        const uint32_t temp_raw = (uint32_t)(((t_raw[0] << 16) | (t_raw[1] << 8) | t_raw[2]) >> 4);

        printf("TEMP RAW: %d\n", temp_raw);

        const int32_t temp = calculate_temp_from_raw_value(dig_T1, dig_T2, dig_T3, temp_raw);

        printf("TEMP: %d\n", temp);

        vTaskDelay(2000 / portTICK_RATE_MS);
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

        ESP_ERROR_CHECK(esp_wifi_connect());

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
