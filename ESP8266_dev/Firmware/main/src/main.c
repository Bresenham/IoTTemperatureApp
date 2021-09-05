#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

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

#include <netdb.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#include "../include/utility.h"
#include "../include/bmp280.h"
#include "../include/wifi_config.h"

TaskHandle_t wifi_scan_task_hndl = NULL;
TaskHandle_t wifi_connect_task_hndl = NULL;
TaskHandle_t i2c_bmp280_task_hndl = NULL;

#define WEB_SERVER      "192.168.0.103"
#define WEB_ENDPOINT    "/sensor/add"
#define WEB_PORT        "3000"

static void ICACHE_FLASH_ATTR temp_post_req(uint32_t temp) {

    char request_body[64];
    snprintf(request_body, sizeof(request_body), "{\"id\": 1,\"value\": %d,\"auth_key\": \"\"}", temp);

    char request_buffer[256];
    snprintf(request_buffer, sizeof(request_buffer), "POST %s HTTP/1.1\r\nHOST: %s\r\nContent-Type:application/json\r\nAccept:application/json\r\nContent-Length: %d\r\n\r\n%s\r\n", WEB_ENDPOINT, WEB_SERVER, strlen(request_body), request_body);

    const struct addrinfo addr_inf = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *res;
    struct in_addr *addr;

    int err = getaddrinfo(WEB_SERVER, WEB_PORT, &addr_inf, &res);
    if(err != 0 || res == NULL) {
        printf("DNS lookup failed err=%d res=%p\n", err, res);
        return;
    }

    addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    printf("DNS lookup succeeded. IP=%s\n", inet_ntoa(*addr));

    const int sckt = socket(res->ai_family, res->ai_socktype, 0);
    if(sckt < 0) {
        printf("Failed to allocate socket\n");
        freeaddrinfo(res);
        return;
    }

    if( connect(sckt, res->ai_addr, res->ai_addrlen) != 0 ) {
        printf("Socket failed to connect errno=%d\n", errno);
        close(sckt);
        freeaddrinfo(res);
        return;
    }

    freeaddrinfo(res);

    if( write(sckt, request_buffer, strlen(request_buffer)) < 0 ) {
        printf("Socket send failed\n");
        close(sckt);
    }

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(sckt, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0) {
        printf("Failed to set socket receive timeout!\n");
        close(sckt);
        return;
    }

    printf("Successfully sent request:\n%s\n", request_buffer);

    printf("Received from Socket:\n");
    char recv_buf[128];
    int ret = 0;
    /* Read HTTP response */
    do {
        bzero(recv_buf, sizeof(recv_buf));
        ret = read(sckt, recv_buf, sizeof(recv_buf)-1);
        for(int i = 0; i < ret; i++) {
            putchar(recv_buf[i]);
        }
    } while(ret > 0);

    close(sckt);
}

static void ICACHE_FLASH_ATTR i2c_bmp280_task(void *arg) {

    BMP280 bmp280;
    initBMP280(&bmp280);

    const uint8_t bmp280_id = bmp280.getID();
    printf("BMP280 ID: %hu\n", bmp280_id);

    while(true) {

        const int32_t temp = bmp280.getTemperature(&bmp280);

        printf("T32: %d\n", temp);

        temp_post_req(temp);

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
