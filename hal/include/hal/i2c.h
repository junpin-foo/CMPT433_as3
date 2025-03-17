/* i2c.h 
 * 
 * This file declares functions for initializing and cleaning up the I2C interface, 
 * opening an I2C bus, and performing 16-bit register read/write operations.
 * 
 * These methods are taken from class guide: I2C Guide
 */

#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

// Initializes the I2C interface
void Ic2_initialize(void);

// Cleans up the I2C interface
void Ic2_cleanUp(void);

// Opens an I2C bus with the specified address
int init_i2c_bus(const char* bus, int address);

// Writes a 16-bit value to the specified register
void write_i2c_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value);

// Reads a 16-bit value from the specified register
uint16_t read_i2c_reg16(int i2c_file_desc, uint8_t reg_addr);

// Writes an 8-bit value to the specified register
void write_i2c_reg8(int i2c_file_desc, uint8_t reg_addr, uint8_t value);

// Burst reads from the spcifiec address and length
void read_i2c_burst(int i2c_file_desc, uint8_t reg_addr, uint8_t *buffer, int length);

uint8_t read_i2c_reg8(int i2c_file_desc, uint8_t reg_addr);
#endif