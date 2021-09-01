/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_spi_flash.h"

#include "driver/spi.h"

#include "../include/spi_handler.h"

void ICACHE_FLASH_ATTR main_task(void *arg) {

	while(true) {

		printf("IoT Temperature Sensor started...\r\n");
		vTaskDelay(100 / portTICK_RATE_MS);
	}

}

void app_main() {

	spi_config_t spi_config = {
		.interface.val = SPI_DEFAULT_INTERFACE,
		.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE,
		.mode = SPI_MASTER_MODE,
		.clk_div = SPI_2MHz_DIV,
		.event_cb = &spi_callback
	};

	spi_init(HSPI_HOST, &spi_config);

	
	xTaskCreate(main_task, "main_task", 1024, NULL, 10, NULL);
}
