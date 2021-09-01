#include "../include/spi_handler.h"

#include "driver/spi.h"

void ICACHE_FLASH_ATTR spi_callback(int event, void *arg) {

    switch (event) {
        case SPI_INIT_EVENT: {
			
        } break;

        case SPI_TRANS_START_EVENT: { }
        break;

        case SPI_TRANS_DONE_EVENT: {

        } break;

        case SPI_DEINIT_EVENT: { }
        break;
    }

}