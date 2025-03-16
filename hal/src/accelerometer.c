/* accelerometer.c
 * 
 * This file provides a HAL for interfacing with an
 * accelerometer over I2C using the existing I2C functions.
 */

#include "hal/accelerometer.h"
#include "hal/i2c.h"
#include "beatPlayer.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h> 
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>

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
#define DRUM_THRESHOLD 2000 
#define DEBOUNCE_TIME_MS 130
static int16_t prev_x = 0, prev_y = 0, prev_z = 0;
static struct timespec last_x_time, last_y_time, last_z_time;

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
    write_i2c_reg8(i2c_file_desc, REG_CTRL1, 0x57);  //100Hz, (High)14-bit resolution, (Low)14-bit resolution 
    // write_i2c_reg8(i2c_file_desc, REG_CTRL3, 0x03);  //no sleep
    // write_i2c_reg8(i2c_file_desc, REG_CTRL4, 0x01);  //Data-Ready is routed to INT1 pad
    // write_i2c_reg8(i2c_file_desc, REG_CTRL5, 0x01);
    // write_i2c_reg8(i2c_file_desc, REG_CTRL6, 0x24); //+8g
    // write_i2c_reg8(i2c_file_desc, REG_CTRL6, 0x00); //+2g
    // write_i2c_reg8(i2c_file_desc, REG_CTRL2, 0x40); //soft reset

    // uint8_t fifo_config = (0x00 << 3) | (0x00);  // FMode = 110, FTH = 0x0A
    // write_i2c_reg8(i2c_file_desc, REG_FIFO_CTRL, fifo_config);
}

void Accelerometer_cleanUp(void) {
    keepReading = false;
    pthread_join(accelerometer_thread, NULL);
    // Ic2_cleanUp();
    isInitialized = false;
}

// Calculate time difference in milliseconds
static long time_diff_ms(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000 + (end->tv_nsec - start->tv_nsec) / 1000000;
}

void *Accelerometer_thread_func(void *arg) {
    (void)arg;
    assert(isInitialized);

    clock_gettime(CLOCK_MONOTONIC, &last_x_time);
    clock_gettime(CLOCK_MONOTONIC, &last_y_time);
    clock_gettime(CLOCK_MONOTONIC, &last_z_time);

    while (keepReading) {
        AccelerometerData data = Accelerometer_getReading();

        int16_t dx = abs(data.x - prev_x);
        int16_t dy = abs(data.y - prev_y);
        int16_t dz = abs(data.z - prev_z);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        // printf("dx: %d, dy: %d, dz: %d\n", dx, dy, dz);
        if (dx > 2000  && time_diff_ms(&last_x_time, &now) > DEBOUNCE_TIME_MS) {
            printf("x detected!\n");
            BeatPlayer_playHiHat();
            last_x_time = now;
        }
        if (dy > 2000  && time_diff_ms(&last_y_time, &now) > DEBOUNCE_TIME_MS) {
            printf("y detected!\n");
            BeatPlayer_playSnare();
            last_y_time = now;
        }
        if (dz > 4000 && time_diff_ms(&last_z_time, &now) > DEBOUNCE_TIME_MS) {
            printf("z detected!\n");
            BeatPlayer_playBaseDrum();
            last_z_time = now;
        }

        prev_x = data.x;
        prev_y = data.y;
        prev_z = data.z;
       
        struct timespec reqDelay = {0, 10000000};
        nanosleep(&reqDelay, (struct timespec *) NULL); // 10ms delay
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
    // for (int i = 13; i >= 0; i--) printf("%d", (x >> i) & 1);
    // printf(" (%" PRIx16 "h)\n", x);

    // printf("Y: 0b");
    // for (int i = 13; i >= 0; i--) printf("%d", (y >> i) & 1);
    // printf(" (%" PRIx16 "h)\n", y);

    // printf("Z: 0b");
    // for (int i = 13; i >= 0; i--) printf("%d", (z >> i) & 1);
    // printf(" (%" PRIx16 "h)\n", z);

    // printf("X: %d, Y: %d, Z: %d\n", x, y, z);
    // nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    AccelerometerData data = {x, y, z};

    return data;
}

Period_statistics_t Accelerometer_getSamplingTime() {
    assert(isInitialized);
    Period_statistics_t stats;
    Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_ACCEL, &stats);
    return stats;
}