#ifndef I2C_H
#define I2C_H

#include "stm32f439xx.h"
#include "stdint.h"
#include "stdbool.h"

// Macro definitions for device config literals
//
// clang-format off
#define I2C_CONFIG_DEFAULT      \
    (I2C_Config) {              \
        .I2CEN      = true,     \
        .CLKFREQ    = 42,       \
        .CLKMODE    = I2C_SM,   \
        .CCR        = 210,      \
        .TRISE      = 46        \
    }
// clang-format on

typedef enum {
    I2C_SM,         // Standard mode
    I2C_FM          // Fast mode
}   I2C_ClockMode;

typedef struct {
    bool I2CEN              : 1;
    uint8_t CLKFREQ         : 6;
    I2C_ClockMode CLKMODE   : 1;
    uint16_t CCR            : 11;
    uint8_t TRISE           : 6;
} I2C_Config;

typedef struct I2C {
    I2C_TypeDef *interface;
    I2C_Config config;
    void (*updateConfig)(struct I2C *i2c, I2C_Config *config);
    void (*write)(struct I2C *i2c, uint8_t addr, uint8_t *data, uint8_t size);
    void (*read)(struct I2C *i2c, uint8_t addr, uint8_t *buf, uint8_t size);
} I2C_t;

I2C_t I2C_init(I2C_TypeDef *interface, I2C_Config *config);
void I2C_reset(I2C_t *i2c);
void I2C_start(I2C_t *i2c);
void I2C_sendAddress(I2C_t *i2c, uint8_t addr);
void I2C_stop(I2C_t *i2c);
void I2C_waitUntilReady(I2C_t *i2c, uint8_t addr);

void I2C_updateConfig(I2C_t *i2c, I2C_Config *config);
bool I2C_pollAck(I2C_t *i2c, uint8_t addr);
void I2C_write(I2C_t *i2c, uint8_t addr, uint8_t *data, uint8_t size);
void I2C_readByte(I2C_t *i2c, uint8_t addr, uint8_t *buf);
void I2C_readBytes(I2C_t *i2c, uint8_t addr, uint8_t *buf, uint8_t size);

#endif