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

#include "../include/utility.h"
#include "../include/bmp280.h"
#include "../include/wifi_config.h"

#define I2C_MASTER_SDA_GPIO                         GPIO_NUM_4
#define I2C_MASTER_SCK_GPIO                         GPIO_NUM_5

#define BMP280_I2C_WRITE_ADDR                       (0b11101110)
#define BMP280_I2C_READ_ADDR                        (0b11101111)

#define BMP280_SLAVE_ACK_EN                         (true)

TaskHandle_t wifi_scan_task_hndl = NULL;
TaskHandle_t wifi_connect_task_hndl = NULL;
TaskHandle_t i2c_bmp280_task_hndl = NULL;

static void ICACHE_FLASH_ATTR delay_ms(uint32_t period) {

    vTaskDelay(period / portTICK_RATE_MS);

}

static int8_t ICACHE_FLASH_ATTR i2c_bmp280_read_reg(uint8_t dev_id, uint8_t reg_start_addr, uint8_t *data, uint16_t read_length) {

    i2c_cmd_handle_t read_cmd = i2c_cmd_link_create();
    i2c_master_start(read_cmd);
    i2c_master_write_byte(read_cmd, BMP280_I2C_WRITE_ADDR, BMP280_SLAVE_ACK_EN);
    i2c_master_write_byte(read_cmd, reg_start_addr, BMP280_SLAVE_ACK_EN);

    i2c_master_start(read_cmd);
    i2c_master_write_byte(read_cmd, BMP280_I2C_READ_ADDR, BMP280_SLAVE_ACK_EN);

    for(uint16_t i = 0; i < read_length; ++i) {

        if( i == read_length - 1 ) {
            i2c_master_read(read_cmd, &data[i], 1, I2C_MASTER_NACK);
        } else {
            i2c_master_read(read_cmd, &data[i], 1, I2C_MASTER_ACK);
        }
    }

    const esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, read_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(read_cmd);

    if(ret != ESP_OK) {
        printf("FAILED I2C BMP280 READ CMD!\n");
        return -1;
    }

    return 0;
}

static int8_t ICACHE_FLASH_ATTR i2c_bmp280_write_reg(uint8_t dev_id, uint8_t reg_start_addr, uint8_t *data, uint16_t length) {

    i2c_cmd_handle_t write_cmd = i2c_cmd_link_create();
    i2c_master_start(write_cmd);
    i2c_master_write_byte(write_cmd, BMP280_I2C_WRITE_ADDR, BMP280_SLAVE_ACK_EN);

    for(uint16_t i = 0; i < length; ++i) {
        i2c_master_write_byte(write_cmd, reg_start_addr + i, BMP280_SLAVE_ACK_EN);
        i2c_master_write_byte(write_cmd, data[i], BMP280_SLAVE_ACK_EN);
    }

    i2c_master_stop(write_cmd);

    const esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, write_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(write_cmd);

    if(ret != ESP_OK) {
        printf("FAILED I2C BMP280 WRITE CMD!\n");
        return -1;
    }

    return 0;
}

static void ICACHE_FLASH_ATTR i2c_bmp280_task(void *arg) {

    i2c_config_t i2c_conf;
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = I2C_MASTER_SDA_GPIO;
    i2c_conf.sda_pullup_en = 1;
    i2c_conf.scl_io_num = I2C_MASTER_SCK_GPIO;
    i2c_conf.scl_pullup_en = 1;
    i2c_conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.

    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));

    struct bmp280_config conf;
    struct bmp280_dev bmp;
    bmp.dev_id = BMP280_I2C_WRITE_ADDR;
    bmp.intf = BMP280_I2C_INTF;
    bmp.read = i2c_bmp280_read_reg;
    bmp.write = i2c_bmp280_write_reg;
    bmp.delay_ms = delay_ms;
    if( bmp280_init(&bmp) != BMP280_OK ) {
        printf("[BMP280] Failed to initialize BMP280!\n");
    }

    printf("TEMP CALIBRATION VALUES:\n\tdigT1: %hu\n\tdigT2: %hd\n\tdigT3: %hd\n", bmp.calib_param.dig_t1, bmp.calib_param.dig_t2, bmp.calib_param.dig_t3);

    if( bmp280_get_config(&conf, &bmp) != BMP280_OK ) {
        printf("[BMP280] Failed to read config from BMP280!\n");
    }

    conf.filter = BMP280_FILTER_OFF;
    conf.os_temp = BMP280_OS_2X;
    conf.os_pres = BMP280_OS_2X;
    conf.odr = BMP280_ODR_2000_MS;
    if( bmp280_set_config(&conf, &bmp) != BMP280_OK ) {
        printf("[BMP280] Failed to write new config to BMP280!\n");
    }

    if( bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp) != BMP280_OK ) {
        printf("[BMP280] Failed to set power mode of BMP280!\n");
    }

    double temp = 0.0;
    volatile int32_t temp32 = 0;
    char float_str[6];

    struct bmp280_uncomp_data ucomp_data;

    while(true) {

        bmp280_get_uncomp_data(&ucomp_data, &bmp);
        bmp280_get_comp_temp_32bit(&temp32, ucomp_data.uncomp_temp, &bmp);
        bmp280_get_comp_temp_double(&temp, ucomp_data.uncomp_temp, &bmp);

        dtostrf(temp, 4, 2, float_str);

        printf("UT: %d, T32: %d, T: %s\n", ucomp_data.uncomp_temp, temp32, float_str);

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
            xTaskCreate(i2c_bmp280_task, "i2c_bmp280_task", 1024 * 32, NULL, 1, &i2c_bmp280_task_hndl);
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
