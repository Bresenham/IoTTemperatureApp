/* 
 * Created @ 01.05.2019
 * 
*/
#ifndef BMP280_PRIVATE_H
#define BMP280_PRIVATE_H

#include "driver/gpio.h"

#define I2C_MASTER_SDA_GPIO         GPIO_NUM_4
#define I2C_MASTER_SCL_GPIO         GPIO_NUM_5

#define DELAY                       1

#define ADDRESS_WRITE               0xEE
#define ADDRESS_READ                0xEF

#define REGISTER_ID                 0xD0

#define REGISTER_CTRL_MEAS          0xF4

#define CTRL_MEAS_TEMP_OVERSAMPLING 0xE0
#define CTRL_MEAS_PRESSURE_SKIP     0x00
#define CTRL_MEAS_NORMAL_MODE       0x03

#define REGISTER_CONFIG             0xF5

#define CONFIG_STANDBY_1000MS       0xA0

#define REGISTER_TEMP_MSB           0xFA
#define REGISTER_TEMP_LSB           0xFB
#define REGISTER_TEMP_XLSB          0xFC

#define REGISTER_DIG_T1_MSB         0x89
#define REGISTER_DIG_T1_LSB         0x88

#define REGISTER_DIG_T2_MSB         0x8B
#define REGISTER_DIG_T2_LSB         0x8A

#define REGISTER_DIG_T3_MSB         0x8D
#define REGISTER_DIG_T3_LSB         0x8C

uint8_t ICACHE_FLASH_ATTR bmp280GetID();

int32_t ICACHE_FLASH_ATTR bmp280GetTemperature(BMP280 *self);

#endif /* BMP280_PRIVATE_H */
