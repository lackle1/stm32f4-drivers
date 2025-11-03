#ifndef I2C
#define I2C

#include <stdint.h>
#include "stm32f439xx.h"

void I2C_config(I2C_TypeDef *i2c);
void I2C_reset(I2C_TypeDef *i2c);
void I2C_start(I2C_TypeDef *i2c);
void I2C_sendAddress(I2C_TypeDef *i2c, uint8_t addr);
void I2C_stop(I2C_TypeDef *i2c);
void I2C_waitUntilReady(I2C_TypeDef *i2c, uint8_t addr);
void I2C_write(I2C_TypeDef *i2c, uint8_t addr, uint8_t *data, uint8_t size);
void I2C_read(I2C_TypeDef *i2c, uint8_t addr, uint8_t *buf, uint8_t size);

#endif