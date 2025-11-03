#ifndef EEPROM
#define EEPROM

#include <stdint.h>
#include "stm32f439xx.h"

#define EEPROM_ADDRESS  0b10100000       // 

#define PAGE_NUM        4096
#define PAGE_SIZE       1               // Counted in bytes

void EEPROM_write(I2C_TypeDef *i2c, uint16_t page, uint8_t *data, uint8_t size);
void EEPROM_read(I2C_TypeDef *i2c, uint16_t page, uint8_t *data, uint8_t size);

#endif