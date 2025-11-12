/***********************************************************************************
 * @file        CAT24C32.c                                                              *
 * @author      Lachie Keane                                                       *
 * @addtogroup  CAT24C32                                                                *
 * @brief       Implements functions for I2C device interface initialization and   *
 *              communication.                                                     *
 ***********************************************************************************/

#include "cat24c32.h"
#include "i2c.h"

// TODO: this is just temporary. Change it when this code gets put into Australis
void initCAT24C32() {

    // Enable GPIO and I2C clocks
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  // I2C
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // GPIOB

    // Configure pins PB8 and PB9 to use alternate functions
    GPIOB->MODER |= 2<<GPIO_MODER_MODE8_Pos | 2<<GPIO_MODER_MODE9_Pos;              // Alternative function mode (0b10)
    GPIOB->OTYPER |= GPIO_OTYPER_OT8 & GPIO_OTYPER_OT9;                             // Output open-drain
    GPIOB->OSPEEDR |= 3<<GPIO_OSPEEDR_OSPEED8_Pos | 3<<GPIO_OSPEEDR_OSPEED9_Pos;    // Highspeed (0b11)
    GPIOB->PUPDR |= 1<<GPIO_PUPDR_PUPD8_Pos | 1<<GPIO_PUPDR_PUPD9_Pos;              // Set pins to pull-up

    // Configure which alternate function will be used
    GPIOB->AFR[1] |= 4<<0 | 4<<4;       // Set each to AF4 (I2C)
}

/**
 * @brief  Initialise CAT24C32 EEPROM device
 *
 * @param  cat24c32 Pointer to device struct to be initialised
 * @param  i2c      Pointer to I2C_t struct
 * @param  config   Pointer to config struct 
 *
 * @return Initialised CAT24C32 struct
 **/
bool CAT24C32_init(CAT24C32_t *cat24c32, I2C_t *i2c, CAT24C32_Config *config) {

    bool initialised = true;

    cat24c32->updateConfig = CAT24C32_updateConfig;
    cat24c32->write = CAT24C32_write;
    cat24c32->read = CAT24C32_read;

    if (i2c != NULL)
        cat24c32->i2c = *i2c;
    else
        initialised = false;

    return initialised && cat24c32->updateConfig(cat24c32, config);
}

bool CAT24C32_updateConfig(CAT24C32_t *cat24c32, CAT24C32_Config *config) {

    bool configured = true;

    if (config == NULL)
        configured = false;

    cat24c32->config = *config;


    return configured;
}

/**
 * @brief  Writes up to 32 bytes of data to the CAT24C32 device
 *
 * @param  page Start page
 * @param  data Data to be written
 * @param  size Number of bytes to be written, up to 32
 *
 * @return @c NULL
 **/
void CAT24C32_write(CAT24C32_t *cat24c32, uint16_t page, uint8_t *data, size_t size) {

    I2C_waitUntilReady(&cat24c32->i2c, cat24c32->config.address & 0xFE);     // LSB cleared to select write mode

    // Send address of memory location within the CAT24C32 device
    uint8_t addr[2];
    addr[0] = page >> 8;    // Higher byte
    addr[1] = page;         // Lower byte
    I2C_write(&cat24c32->i2c, cat24c32->config.address, addr, 2);

    // Send the data we want to write
    I2C_write(&cat24c32->i2c, cat24c32->config.address, data, size);

    I2C_stop(&cat24c32->i2c);
}

void CAT24C32_read(CAT24C32_t *cat24c32, uint16_t page, uint8_t *data, size_t size) {

    I2C_waitUntilReady(&cat24c32->i2c, CAT24C32_ADDRESS);

    // Send address of memory location within the CAT24C32 device
    uint8_t addr[2];
    addr[0] = page >> 8;    // Higher byte
    addr[1] = page;         // Lower byte
    I2C_write(&cat24c32->i2c, cat24c32->config.address, addr, 2);

    // Restart and set to read mode
    I2C_stop(&cat24c32->i2c);
    I2C_start(&cat24c32->i2c);
    I2C_sendAddress(&cat24c32->i2c, cat24c32->config.address | 0x01);  // LSB set to select read mode

    I2C_read(&cat24c32->i2c, CAT24C32_ADDRESS, data, size);

    I2C_stop(&cat24c32->i2c);
}