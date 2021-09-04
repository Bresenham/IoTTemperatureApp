/* 
 * Created @ 01.05.2019
 * 
*/
#ifndef BMP280_H
#define BMP280_H

#include "stdint.h"

#define BMP280_EXPECTED_ID  0x58

typedef struct BMP280 {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;


    uint8_t (*getID)();
    int32_t (*getTemperature)(struct BMP280*);
} BMP280;

void ICACHE_FLASH_ATTR initBMP280(BMP280 *self);

#endif /* BMP280_H */
