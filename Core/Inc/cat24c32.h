#ifndef CAT24C32_H
#define CAT24C32_H

#include "stm32f439xx.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"

#include "i2c.h"

#define CAT24C32_ADDRESS  0xA0 

#define CAT24C32_STORAGE_CAPACITY 4096
#define CAT24C32_PAGE_NUM         4096
#define CAT24C32_PAGE_SIZE        1           // Counted in bytes

#define CAT24C32_MAX_WRITE_SIZE   32

typedef struct {

} GPIOpin_t;

typedef struct {
    uint8_t address;    // I2C device address
    GPIOpin_t wp;       // Write protect GPIO
} CAT24C32_Config;

typedef struct CAT24C32 {
    I2C_t i2c;
    CAT24C32_Config config;
    bool (*updateConfig)(struct CAT24C32 *cat24c32, CAT24C32_Config *config);
    void (*write)(struct CAT24C32 *cat24c32, uint16_t page, uint8_t *data, size_t size);
    void (*write32)(struct CAT24C32 *cat24c32, uint16_t page, uint8_t *data, size_t size);
    void (*read)(struct CAT24C32 *cat24c32, uint16_t page, uint8_t *data, size_t size);
} CAT24C32_t;

void initCAT24C32();

bool CAT24C32_init(CAT24C32_t *cat24c32, I2C_t *i2c, CAT24C32_Config *config);
bool CAT24C32_updateConfig(CAT24C32_t *cat24c32, CAT24C32_Config *config);
void CAT24C32_write(CAT24C32_t *cat24c32, uint16_t page, uint8_t *data, size_t size);
void CAT24C32_write32(CAT24C32_t *cat24c32, uint16_t page, uint8_t *data, size_t size);
void CAT24C32_read(CAT24C32_t *cat24c32, uint16_t page, uint8_t *data, size_t size);
void CAT24C32_erase(CAT24C32_t *cat24c32);

#endif