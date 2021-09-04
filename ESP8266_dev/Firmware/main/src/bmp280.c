#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "../include/bmp280.h"
#include "../include/bmp280_private.h"

/********************/
/* PRIVATE FUNCTIONS */
/********************/
static uint8_t ICACHE_FLASH_ATTR read_SDA(void) {
    return gpio_get_level(I2C_MASTER_SDA_GPIO);
}

static void ICACHE_FLASH_ATTR set_SDA_high(void) {
    gpio_set_level(I2C_MASTER_SDA_GPIO, 1);
}

static void ICACHE_FLASH_ATTR set_SDA_low(void) {
    gpio_set_level(I2C_MASTER_SDA_GPIO, 0);
}

static void ICACHE_FLASH_ATTR set_SCL_high(void) {
    gpio_set_level(I2C_MASTER_SCL_GPIO, 1);
}

static void ICACHE_FLASH_ATTR set_SCL_low(void) {
    gpio_set_level(I2C_MASTER_SCL_GPIO, 0);
}

static void ICACHE_FLASH_ATTR i2c_master_nack() {
    set_SCL_low();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    set_SCL_high();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    set_SCL_low();
}

static void ICACHE_FLASH_ATTR i2c_master_ack() {
    set_SDA_low();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    set_SCL_high();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    set_SCL_low();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    set_SDA_high();
}

static void ICACHE_FLASH_ATTR i2c_start() {
    set_SDA_low();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    set_SCL_low();
    vTaskDelay(DELAY / portTICK_RATE_MS);
}

static void ICACHE_FLASH_ATTR i2c_stop() {
    set_SCL_high();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    set_SDA_high();
    vTaskDelay(DELAY / portTICK_RATE_MS);
}

static void ICACHE_FLASH_ATTR init_i2c_ports() {

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = (1UL << I2C_MASTER_SDA_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = (1UL << I2C_MASTER_SCL_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    set_SDA_high();
    set_SCL_high();

    vTaskDelay(10 * DELAY / portTICK_RATE_MS);
}

static uint8_t ICACHE_FLASH_ATTR i2c_read_byte() {

    uint8_t read_byte = 0x00;
    set_SDA_high();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    for(int8_t i = 7; i >= 0; i--) {
        vTaskDelay(DELAY / portTICK_RATE_MS);
        set_SCL_high();
        vTaskDelay(DELAY / portTICK_RATE_MS);
        const uint8_t sda_val = read_SDA();
        if(sda_val == 1) {
            read_byte |= (1 << i);
        }
        set_SCL_low();
    }

    return read_byte;
}

static bool ICACHE_FLASH_ATTR i2c_send_byte(uint8_t byte) {

    for(int8_t i = 7; i >= 0; i--) {
        if(byte & (1 << i))
            set_SDA_high();
        else
            set_SDA_low();

        vTaskDelay(DELAY / portTICK_RATE_MS);
        set_SCL_high();
        vTaskDelay(DELAY / portTICK_RATE_MS);
        set_SCL_low();
    }
    set_SDA_high();
    vTaskDelay(DELAY / portTICK_RATE_MS);

    /* generate 9th clock for slave to ack */
    set_SCL_high();
    vTaskDelay(DELAY / portTICK_RATE_MS);
    const uint8_t slave_ack = read_SDA();
    set_SCL_low();
    set_SDA_low();

    vTaskDelay(DELAY / portTICK_RATE_MS);
    
    return slave_ack == 0;
}

static void ICACHE_FLASH_ATTR get_reg_content_continous(uint8_t reg_start_addr, uint8_t amount, uint8_t *storage) {

    i2c_start();

    if(i2c_send_byte(ADDRESS_WRITE)) {
        if(i2c_send_byte(reg_start_addr)){
            /* stop condition */
            set_SCL_high();
            vTaskDelay(DELAY / portTICK_RATE_MS);
            set_SDA_high();
            vTaskDelay(DELAY / portTICK_RATE_MS);
            i2c_start();
            if(i2c_send_byte(ADDRESS_READ)) {
                for(uint8_t i = 0; i < amount; i++) {
                    const bool is_last_reg = i == amount - 1;

                    storage[i] = i2c_read_byte();

                    if(is_last_reg)
                        i2c_master_nack();
                    else
                        i2c_master_ack();
                    vTaskDelay(DELAY / portTICK_RATE_MS);
                }
            }
        }
    }

    i2c_stop();
}

static uint8_t ICACHE_FLASH_ATTR ICACHE_FLASH_ATTR get_reg_content(uint8_t reg_addr) {

    uint8_t single_reg_data;
    get_reg_content_continous(reg_addr, 1, &single_reg_data);

    return single_reg_data;
}

static void ICACHE_FLASH_ATTR ICACHE_FLASH_ATTR write_reg_content(uint8_t reg_addr, uint8_t reg_content) {

    i2c_start();

    if(i2c_send_byte(ADDRESS_WRITE)) {
        if(i2c_send_byte(reg_addr)) {
            if(i2c_send_byte(reg_content)) {

            }
        }
    }

    i2c_stop();
}

static int32_t ICACHE_FLASH_ATTR calculate_temp_from_raw_value(int16_t dig_T1, uint16_t dig_T2, uint16_t dig_T3, int32_t value) {

    const int32_t var1 = ((((value / 8) - ((int32_t) dig_T1 << 1))) * ((int32_t) dig_T2)) / 2048;
    const int32_t var2 = (((((value / 16) - ((int32_t) dig_T1)) * ((value / 16) - ((int32_t) dig_T1))) / 4096) * ((int32_t) dig_T3)) / 16384;
    const int32_t t_fine = var1 + var2;
    const int32_t comp_temp = (t_fine * 5 + 128) / 256;
    
    return comp_temp;
}

static void ICACHE_FLASH_ATTR load_trimming_values(BMP280 *self) {

    const uint8_t dig_T1_LSB = get_reg_content(REGISTER_DIG_T1_LSB);
    const uint8_t dig_T1_MSB = get_reg_content(REGISTER_DIG_T1_MSB);
    const uint16_t dig_T1 = (dig_T1_MSB << 8) | dig_T1_LSB;

    const uint8_t dig_T2_LSB = get_reg_content(REGISTER_DIG_T2_LSB);
    const uint8_t dig_T2_MSB = get_reg_content(REGISTER_DIG_T2_MSB);
    const int16_t dig_T2 = (int16_t)((dig_T2_MSB << 8) | dig_T2_LSB);

    const uint8_t dig_T3_LSB = get_reg_content(REGISTER_DIG_T3_LSB);
    const uint8_t dig_T3_MSB = get_reg_content(REGISTER_DIG_T3_MSB);
    const int16_t dig_T3 = (int16_t)((dig_T3_MSB << 8) | dig_T3_LSB);

    self->dig_T1 = dig_T1;
    self->dig_T2 = dig_T2;
    self->dig_T3 = dig_T3;
}

/********************/
/* PUBLIC FUNCTIONS */
/********************/

int32_t ICACHE_FLASH_ATTR bmp280GetTemperature(BMP280 *self) {

    uint8_t t_raw[3];
    get_reg_content_continous(REGISTER_TEMP_MSB, 3, t_raw);

    const int32_t value = (int32_t) ((((int32_t) (t_raw[0])) << 12) | (((int32_t) (t_raw[1])) << 4) | (((int32_t) (t_raw[2])) >> 4));

    return calculate_temp_from_raw_value(self->dig_T1, self->dig_T2, self->dig_T3, value);
}

uint8_t ICACHE_FLASH_ATTR bmp280GetID() {

    return get_reg_content(REGISTER_ID);
}

void ICACHE_FLASH_ATTR initBMP280(BMP280 *self) {

    init_i2c_ports();
    write_reg_content(REGISTER_CTRL_MEAS, CTRL_MEAS_TEMP_OVERSAMPLING | CTRL_MEAS_PRESSURE_SKIP | CTRL_MEAS_NORMAL_MODE);
    write_reg_content(REGISTER_CONFIG, CONFIG_STANDBY_1000MS);

    load_trimming_values(self);

    self->getID = &bmp280GetID;
    self->getTemperature = &bmp280GetTemperature;
}
