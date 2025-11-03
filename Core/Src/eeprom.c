/***********************************************************************************
 * @file        eeprom.c                                                              *
 * @author      Lachie Keane                                                       *
 * @addtogroup  EEPROM                                                                *
 * @brief       Implements functions for I2C device interface initialization and   *
 *              communication.                                                     *
 ***********************************************************************************/

#include "eeprom.h"
#include "i2c.h"

/**
 * @brief  Writes up to 32 bytes of data to the EEPROM device
 *
 * @param  page Start page
 * @param  data Data to be written
 * @param  size Number of bytes to be written, up to 32
 *
 * @return @c NULL
 **/
void EEPROM_write(I2C_TypeDef *i2c, uint16_t page, uint8_t *data, uint8_t size) {

    // Begin transfer
    // I2C_start(i2c);
    // I2C_sendAddress(i2c, EEPROM_ADDRESS);   // Last bit is left as 0 to set write mode.

    I2C_waitUntilReady(i2c, EEPROM_ADDRESS);

    // Send address of memory location within the EEPROM device
    uint8_t addr[2];
    addr[0] = page >> 8;    // Higher byte
    addr[1] = page;         // Lower byte
    I2C_write(i2c, EEPROM_ADDRESS, addr, 2);

    // Send the data we want to write
    I2C_write(i2c, EEPROM_ADDRESS, data, size);

    I2C_stop(i2c);
}

void EEPROM_read(I2C_TypeDef *i2c, uint16_t page, uint8_t *data, uint8_t size) {

    // Begin transfer
    // I2C_start(i2c);
    // I2C_sendAddress(i2c, EEPROM_ADDRESS);   // Last bit is left as 0 to set write mode.

    I2C_waitUntilReady(i2c, EEPROM_ADDRESS);

    // Send address of memory location within the EEPROM device
    uint8_t addr[2];
    addr[0] = page >> 8;    // Higher byte
    addr[1] = page;         // Lower byte
    I2C_write(i2c, EEPROM_ADDRESS, addr, 2);

    // Restart and set to read mode
    I2C_stop(i2c);
    I2C_start(i2c);
    I2C_sendAddress(i2c, EEPROM_ADDRESS | 1);  // Make sure LSB is 1 for read

    I2C_read(i2c, EEPROM_ADDRESS, data, size);

    I2C_stop(i2c);
}