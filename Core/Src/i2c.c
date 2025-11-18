/***********************************************************************************
 * @file        i2c.c                                                              *
 * @author      Lachie Keane                                                       *
 * @addtogroup  I2C                                                                *
 * @brief       Implements functions for I2C interface initialization and          *
 *              communication.                                                     *
 ***********************************************************************************/

#include "i2c.h"
#include "stdlib.h"

static void _I2C_init(I2C_TypeDef *interface, I2C_Config *config);

/**
 * @brief  Initialises I2C
 *
 * @param  interface    Pointer to the I2C_t struct.
 * @param  config       Pointer to I2C_Config struct for initial configuration.
 *                      This may be \c NULL, in which case the default will be used.
 *
 * @return Initialised I2C_t struct;
 **/
I2C_t I2C_init(I2C_TypeDef *interface, I2C_Config *config) {
    
    // Early return error struct if peripheral is null
    if (interface == NULL)
        return (I2C_t){.interface = NULL};

    I2C_t i2c = {.interface = interface};

    I2C_updateConfig(&i2c, config);

    i2c.updateConfig = I2C_updateConfig;
    i2c.read = I2C_read;
    i2c.write = I2C_write;

    return i2c;
}

/**
 * @brief   Updates I2C configuration
 * @details Uses the config struct to update the I2C registers and resets the peripheral.
 *          Passing \c NULL uses the default config
 *
 * @param  i2c      Pointer to the I2C_t struct.
 * @param  config   Pointer to config struct
 *
 * @return @c NULL
 **/
void I2C_updateConfig(I2C_t *i2c, I2C_Config *config) {

    // Use default values if config is null
    if (config == NULL) {
        config = &I2C_CONFIG_DEFAULT;
    }

    i2c->config = *config;

    // Initialise I2C registers and enable peripheral
    _I2C_init(i2c->interface, config);
}

/**
 * @brief  Updates I2C registers and enables the peripheral
 *
 * @param  interface Pointer struct representing the I2C interface
 * @param  config    Pointer to config struct
 *
 * @return @c NULL
 **/
static void _I2C_init(I2C_TypeDef *interface, I2C_Config *config) {

    // Wait until bus is not busy
    while (interface->SR2 & I2C_SR2_BUSY);

    // Reset I2C (TODO: not 100% sure if this needed)
    interface->CR1 |= I2C_CR1_SWRST;
    interface->CR1 &= ~I2C_CR1_SWRST;
    
    // Disable I2C
    interface->CR1 &= ~I2C_CR1_PE;

    // Configure clock frequency
    interface->CR2 &= ~I2C_CR2_FREQ;
    interface->CR2 |= config->CLKFREQ << I2C_CR2_FREQ_Pos;

    // Configure clock control register
    interface->CCR &= ~I2C_CCR_FS;
    interface->CCR |= config->CLKMODE << I2C_CCR_FS_Pos;
    
    interface->CCR &= ~I2C_CCR_CCR;
    interface->CCR |= config->CCR << I2C_CCR_CCR_Pos;

    // Configure TRISE
    interface->TRISE &= ~I2C_TRISE_TRISE;
    interface->TRISE |= config->TRISE << I2C_TRISE_TRISE_Pos;
    
    // Re-enable I2C
    interface->CR1 |= I2C_CR1_PE;
}

/**
 * @brief  Resets I2C
 *
 * @param  i2c Pointer to I2C_t struct.
 *
 * @return @c NULL
 **/
void I2C_reset(I2C_t *i2c) {
    i2c->interface->CR1 |= I2C_CR1_SWRST;
    i2c->interface->CR1 &= ~I2C_CR1_SWRST;

    i2c->interface->CR1 &= ~I2C_CR1_PE;  // Disable
    i2c->interface->CR1 |= I2C_CR1_PE;   // Re-enable
}

/**
 * @brief  Prepares for I2C transmission
 *
 * @param  i2c Pointer to I2C_t struct.
 *
 * @return @c NULL
 **/
void I2C_start(I2C_t *i2c) {
    while (i2c->interface->SR2 & I2C_SR2_BUSY);        // Wait for BUSY bit to clear

    i2c->interface->CR1 |= I2C_CR1_ACK;                // Enable ACK
    i2c->interface->CR1 |= I2C_CR1_START;              // Set START bit
    while (!(i2c->interface->SR1 & I2C_SR1_SB));       // Wait for the SB bit to be set
}

/**
 * @brief  Sets the 7-bit address of the slave
 *
 * @param  i2c  Pointer to I2C_t struct.
 * @param  addr Device address
 *  
 * @return @c NULL
 **/
void I2C_sendAddress(I2C_t *i2c, uint8_t addr) {
    i2c->interface->DR = addr;
    while (!(i2c->interface->SR1 & I2C_SR1_ADDR));     // Wait for ADDR bit to be set
    (void)(i2c->interface->SR1 | i2c->interface->SR2);            // Read status registers to clear ADDR
}

/**
 * @brief  Stops transmission
 *
 * @param  i2c  Pointer to I2C_t struct.
 *  
 * @return @c NULL
 **/
void I2C_stop(I2C_t *i2c) {
    i2c->interface->CR1 |= I2C_CR1_STOP;           // Sets stop bit
}

/**
 * @brief   Checks if device acknowledges the address.
 * @details The communication is only stopped if the device NACKs, so the I2C does not need to be started again if the function returns true.
 *
 * @param  i2c  Pointer to the I2C_t struct.
 * @param  addr Device address.
 *
 * @return True if the device ACKs, false if not.
 **/
bool I2C_pollAck(I2C_t *i2c, uint8_t addr) {
    
    // Start and send address
    I2C_start(i2c);
    i2c->interface->DR = addr;

    while (!(i2c->interface->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)));  // Wait for either ACK or NACK
    // bool receivedAck = (i2c->interface->SR1 & I2C_SR1_ADDR) ? true : false;

    if (i2c->interface->SR1 & I2C_SR1_ADDR) {
        (void)(i2c->interface->SR1 | i2c->interface->SR2);         // Read status registers to clear ADDR
        return true;
    }
    else {
        i2c->interface->SR1 &= ~I2C_SR1_AF;
        I2C_stop(i2c);
        return false;
    }
}

/**
 * @brief  Sends data to the device at the address
 *
 * @param  i2c  Pointer to I2C_t struct.
 * @param  addr Device address
 * @param  data Pointer to the beginning of the data
 * @param  size Number of bytes to be sent
 *  
 * @return @c NULL
 **/
void I2C_write(I2C_t *i2c, uint8_t *data, uint8_t size, bool stop) {

    for (int i = 0; i < size; i++) {
        while (!(i2c->interface->SR1 & I2C_SR1_TXE));    // Wait for TxE bit to be set (data register empty)
        i2c->interface->DR = data[i];
    }

    while (!(i2c->interface->SR1 & I2C_SR1_BTF));        // Wait for BTF to be set (byte transfer finished)

    if (stop) {
        I2C_stop(i2c);
    }
}

/**
 * @brief  Reads from I2C device. Also stops the transaction.
 *
 * @param  i2c  Pointer to I2C_t struct representing the I2C i2c->interface
 * @param  addr Device address
 * @param  buf  Buffer where the data will be written
 * @param  size Number of bytes to be read
 *  
 * @return @c NULL
 **/
void I2C_read(I2C_t *i2c, uint8_t deviceAddr, uint8_t *buf, uint8_t size, bool stop) {

    I2C_start(i2c);
    I2C_sendAddress(i2c, deviceAddr);

    if (size == 1) {
        I2C_readByte(i2c, buf, stop);
    }
    else {
        I2C_readBytes(i2c, buf, size, stop);
    }
}

/**
 * @brief  Reads a singular byte. Also stops the transaction.
 *
 * @param  i2c  Pointer to I2C_t struct representing the I2C i2c->interface
 * @param  buf  Buffer where the data will be written
 *  
 * @return @c NULL
 **/
void I2C_readByte(I2C_t *i2c, uint8_t *buf, bool stop) {

    i2c->interface->CR1 &= ~(I2C_CR1_ACK);                     // Disable ACK
    i2c->interface->CR1 |= I2C_CR1_POS;                        // Set POS bit
    
    I2C_stop(i2c);                                             // STOP

    while (!(i2c->interface->SR1 & I2C_SR1_RXNE));             // Wait until RxNE is set (data register not empty)
    buf[0] = i2c->interface->DR;
}

/**
 * @brief  Reads a singular byte. Also stops the transaction.
 *
 * @param  i2c  Pointer to I2C_t struct representing the I2C i2c->interface
 * @param  buf  Buffer where the data will be written
 * @param  size Number of bytes to be read
 *  
 * @return @c NULL
 **/
void I2C_readBytes(I2C_t *i2c, uint8_t *buf, uint8_t size, bool stop) {

    for (int i = 0; i < size - 2; i++) {
        while (!(i2c->interface->SR1 & I2C_SR1_RXNE));         // Wait until RxNE is set (data register not empty)
        buf[i] = i2c->interface->DR;
        i2c->interface->CR1 |= I2C_CR1_ACK;                    // Enable ACK (to acknowledge data has been received)
    }

    // Read second last byte
    while (!(i2c->interface->SR1 & I2C_SR1_RXNE));             // Wait until RxNE is set (data register not empty)
    buf[size - 2] = i2c->interface->DR;

    i2c->interface->CR1 &= ~I2C_CR1_ACK;                       // Disable ACK

    if (stop) {
        I2C_stop(i2c);
    }
    else {
        i2c->interface->CR1 |= I2C_CR1_START;                   // Set START bit. Function does other stuff, so don't use it
    }
    

    // Read last byte
    while (!(i2c->interface->SR1 & I2C_SR1_RXNE));             // Wait until RxNE is set (data register not empty)
    buf[size - 1] = i2c->interface->DR;
}