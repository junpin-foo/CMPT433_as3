/* accelerometer.h 
 * 
 * This file declares functions for initializing and cleaning up the I2C interface, 
 * opening an I2C bus, and performing 16-bit register read/write operations.
 * 
 * These methods are taken from class guide: I2C Guide
 */

#ifndef _ACCELEROMETER_H_
#define _ACCELEROMETER_H_

#include <stdint.h>
#include "periodTimer.h"
typedef struct {
    double x;
    double y;
    double z;
} AccelerometerData;

// Initialize the I2C interface and open the I2C bus. And start the thread to read the accelerometer data.
void Accelerometer_initialize(void);

// Clean up the I2C interface and close the I2C bus.
void Accelerometer_cleanUp(void);

// Get the current accelerometer reading.
AccelerometerData Accelerometer_getReading();

// Get the sampling time of the accelerometer.
Period_statistics_t Accelerometer_getSamplingTime();
#endif