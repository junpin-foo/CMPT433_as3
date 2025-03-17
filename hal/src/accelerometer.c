/* accelerometer.c
 * 
 * This file provides a HAL for interfacing with an
 * accelerometer over I2C using the existing I2C functions.
 */

#include "hal/accelerometer.h"
#include "hal/i2c.h"
#include "beatPlayer.h"
#include "sleep_timer_helper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h> 
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#define I2C_BUS "/dev/i2c-1"
#define ACCEL_I2C_ADDRESS 0x19

// Register addresses (example for an LIS3DH accelerometer)
#define REG_WHO_AM_I 0x0F   // WHO_AM_I register address

#define REG_CTRL1  0x20
#define REG_CTRL2  0x21
#define REG_CTRL3  0x22
#define REG_CTRL4  0x23
#define REG_CTRL5  0x24
#define REG_CTRL6  0x25
#define REG_FIFO_CTRL 0x2E

#define REG_OUT_X_L 0x28
#define REG_OUT_X_H 0x29
#define REG_OUT_Y_L 0x2A
#define REG_OUT_Y_H 0x2B
#define REG_OUT_Z_L 0x2C
#define REG_OUT_Z_H 0x2D

//DEBOUNCE TIMER
#define xy_THRESHOLD 0.4 
#define z_THRESHOLD 0.55 
#define DEBOUNCE_TIME_MS 130

static double prev_x = 0.0, prev_y = 0.0, prev_z = 0.0;
static struct timespec last_x_time, last_y_time, last_z_time;

#define SENSITIVITY_2G 4096.0  // Sensitivity for ±2g range (14-bit resolution)

static int i2c_file_desc = -1;
static bool isInitialized = false;
static volatile bool keepReading = false;
static pthread_t accelerometer_thread;

//PROTOTYPES
void Accelerometer_initialize(void);
void Accelerometer_cleanUp(void);
void *Accelerometer_thread_func(void *arg);
AccelerometerData Accelerometer_getReading(void);

void Accelerometer_initialize(void) {
    Ic2_initialize();
    i2c_file_desc = init_i2c_bus(I2C_BUS, ACCEL_I2C_ADDRESS);
    isInitialized = true;
    keepReading = true;
    pthread_create(&accelerometer_thread, NULL, Accelerometer_thread_func, NULL);

    // Example: Configure accelerometer 
    write_i2c_reg8(i2c_file_desc, REG_CTRL1, 0x97);  //100Hz, (High)14-bit resolution, (Low)14-bit resolution 
    // write_i2c_reg8(i2c_file_desc, REG_CTRL3, 0x03);  //no sleep
    // write_i2c_reg8(i2c_file_desc, REG_CTRL4, 0x01);  //Data-Ready is routed to INT1 pad
    // write_i2c_reg8(i2c_file_desc, REG_CTRL5, 0x01);
    // write_i2c_reg8(i2c_file_desc, REG_CTRL6, 0x04); 
    write_i2c_reg8(i2c_file_desc, REG_CTRL6, 0x00); //+2g
    // write_i2c_reg8(i2c_file_desc, REG_CTRL2, 0x40); //soft reset
    // uint8_t fifo_config = (0x00 << 3) | (0x00);  // FMode = 110, FTH = 0x0A
    // write_i2c_reg8(i2c_file_desc, REG_FIFO_CTRL, fifo_config);

    // uint8_t value = read_i2c_reg16(i2c_file_desc, REG_CTRL6);
    // printf("value: 0b");
    // for (int i = 15; i >= 0; i--) printf("%d", (value >> i) & 1);
    // printf(" (%" PRIx16 "h)\n", value);
}

void Accelerometer_cleanUp(void) {
    keepReading = false;
    pthread_join(accelerometer_thread, NULL);
    // Ic2_cleanUp();
    isInitialized = false;
}


void *Accelerometer_thread_func(void *arg) {
    (void)arg;
    assert(isInitialized);

    clock_gettime(CLOCK_MONOTONIC, &last_x_time);
    clock_gettime(CLOCK_MONOTONIC, &last_y_time);
    clock_gettime(CLOCK_MONOTONIC, &last_z_time);

    while (keepReading) {
        AccelerometerData data = Accelerometer_getReading();

        double dx = fabs(data.x - prev_x);
        double dy = fabs(data.y - prev_y);
        double dz = fabs(data.z - prev_z);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        // printf("dx: %f, dy: %f, dz: %f\n", dx, dy, dz);
        if (dx > xy_THRESHOLD  && time_diff_ms(&last_x_time, &now) > DEBOUNCE_TIME_MS) {
            // printf("x detected!\n");
            BeatPlayer_playHiHat();
            last_x_time = now;
        }
        if (dy > xy_THRESHOLD  && time_diff_ms(&last_y_time, &now) > DEBOUNCE_TIME_MS) {
            // printf("y detected!\n");
            BeatPlayer_playSnare();
            last_y_time = now;
        }
        if (dz > z_THRESHOLD && time_diff_ms(&last_z_time, &now) > DEBOUNCE_TIME_MS) {
            // printf("z detected!\n");
            BeatPlayer_playBaseDrum();
            last_z_time = now;
        }

        prev_x = data.x;
        prev_y = data.y;
        prev_z = data.z;
       
       sleepForMs(10);
    }
    return NULL;
}

AccelerometerData Accelerometer_getReading(void) {
    if (!isInitialized) {
        fprintf(stderr, "Error: Accelerometer not initialized!\n");
        exit(EXIT_FAILURE);
    }

    uint8_t raw_data[6];
    Period_markEvent(PERIOD_EVENT_SAMPLE_ACCEL);
    read_i2c_burst(i2c_file_desc, REG_OUT_X_L, raw_data, 6);

    int16_t x = (int16_t)((raw_data[1] << 8) | raw_data[0]) >> 2;
    int16_t y = (int16_t)((raw_data[3] << 8) | raw_data[2]) >> 2;
    int16_t z = (int16_t)((raw_data[5] << 8) | raw_data[4]) >> 2;

    // Print in binary
    // printf("X: 0b");
    // for (int i = 15; i >= 0; i--) printf("%d", (x >> i) & 1);
    // printf(" (%" PRIx16 "h)\n", x);

    // printf("Y: 0b");
    // for (int i = 15; i >= 0; i--) printf("%d", (y >> i) & 1);
    // printf(" (%" PRIx16 "h)\n", y);

    // printf("Z: 0b");
    // for (int i = 15; i >= 0; i--) printf("%d", (z >> i) & 1);
    // printf(" (%" PRIx16 "h)\n", z);

    // printf("X: %d, Y: %d, Z: %d\n", x, y, z);
    sleepForMs(10);
    AccelerometerData data = {x / SENSITIVITY_2G, y / SENSITIVITY_2G, z / SENSITIVITY_2G};
    // printf("X: %fg, Y: %fg, Z: %fg\n", data.x, data.y, data.z);
    return data;
}

Period_statistics_t Accelerometer_getSamplingTime() {
    assert(isInitialized);
    Period_statistics_t stats;
    Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_ACCEL, &stats);
    return stats;
}