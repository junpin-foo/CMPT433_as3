// /* joystick.c
//     This file implements joystick functionality using an ADC chip over I2C.
//     It initializes the I2C bus, reads joystick position data from the ADC,
//     scales the raw values to a normalized range (-1 to 1), and provides a function
//     to retrieve the current joystick position.
// */ 

#include "hal/joystick.h"
#include "hal/i2c.h"
#include "hal/gpio.h"
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdatomic.h>

#define I2CDRV_LINUX_BUS "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x48 // ADC chip
#define REG_CONFIGURATION 0x01
#define REG_DATA 0x00
#define TLA2024_CHANNEL_CONF_0 0x83C2 // Configuration for Y-axis
#define TLA2024_CHANNEL_CONF_1 0x83D2 // Configuration for X-axis

#define GPIO_CHIP GPIO_CHIP_2
#define GPIO_LINE 15
struct GpioLine* s_line = NULL;

static int i2c_file_desc = -1;
static bool isInitialized = false;

static atomic_int volume = 50;
static atomic_int page_number = 1;

static pthread_t xy_thread, button_thread;
static volatile bool keepReading = true;

static uint16_t x_min = 18, x_max = 1644;
static uint16_t y_min = 8, y_max = 1635;

void *joystick_xy_thread_func(void *arg);
void *joystick_button_thread_func(void *arg);
static JoystickDirection getJoystickDirection(void);

void Joystick_initialize(void) {
    Ic2_initialize();
    Gpio_initialize();
    s_line = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE);
    i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    
    pthread_create(&xy_thread, NULL, joystick_xy_thread_func, NULL);
    pthread_create(&button_thread, NULL, joystick_button_thread_func, NULL);
    
    isInitialized = true;
}

void Joystick_cleanUp(void) {
    keepReading = false;
    pthread_join(xy_thread, NULL);
    pthread_join(button_thread, NULL);
    
    Ic2_cleanUp();
    Gpio_close(s_line);
    Gpio_cleanup();
    isInitialized = false;
}

void *joystick_xy_thread_func(void *arg) {
    (void)arg;
    assert(isInitialized);
    
    while (keepReading) {
        JoystickDirection data = getJoystickDirection();

        if (data == JOYSTICK_UP) {
            int new_volume = atomic_load(&volume) + 5;
            if (new_volume <= 100) atomic_store(&volume, new_volume);
            printf("UP\n");
        } else if (data == JOYSTICK_DOWN) {
            int new_volume = atomic_load(&volume) - 5;
            if (new_volume >= 0) atomic_store(&volume, new_volume);
            printf("DOWN\n");
        }

        usleep(100000); // 100ms delay
    }
    return NULL;
}

void *joystick_button_thread_func(void *arg) {
    (void)arg;
    assert(isInitialized);

    while (keepReading) {
        struct gpiod_line_bulk events;
        bool buttonFlag = false;
        
        int eventCount = Gpio_waitFor1LineChange(s_line, &events);
        if (eventCount > 0) {
            struct gpiod_line_event event;
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&events, 0);
            if (gpiod_line_event_read(line_handle, &event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }

            if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
                buttonFlag = true;
            }
        }

        if (buttonFlag) {
            int new_page = atomic_load(&page_number) % 3 + 1;
            atomic_store(&page_number, new_page);
            printf("Button pressed, page number: %d\n", new_page);
        }

        usleep(100000); // 100ms delay
    }
    return NULL;
}

static double scale_value(double raw, double min, double max) {
    return 2.0 * ((raw - min) / (max - min)) - 1.0;
}

static JoystickDirection getJoystickDirection(void) {
    struct JoystickData data = Joystick_getReading();
    if (data.x > 0.7) return JOYSTICK_RIGHT;
    if (data.x < -0.7) return JOYSTICK_LEFT;
    if (data.y > 0.7) return JOYSTICK_UP;
    if (data.y < -0.7) return JOYSTICK_DOWN;
    return JOYSTICK_CENTER;
}

struct JoystickData Joystick_getReading() {
    if (!isInitialized) {
        fprintf(stderr, "Error: Joystick not initialized!\n");
        exit(EXIT_FAILURE);
    }

    write_i2c_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_0);
    uint16_t raw_y = read_i2c_reg16(i2c_file_desc, REG_DATA);
    uint16_t y_position = ((raw_y & 0xFF00) >> 8 | (raw_y & 0x00FF) << 8) >> 4;

    if (y_position < y_min) y_min = y_position;
    if (y_position > y_max) y_max = y_position;
    double y_scaled = scale_value(y_position, y_min, y_max) * -1;

    write_i2c_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_1);
    uint16_t raw_x = read_i2c_reg16(i2c_file_desc, REG_DATA);
    uint16_t x_position = ((raw_x & 0xFF00) >> 8 | (raw_x & 0x00FF) << 8) >> 4;

    if (x_position < x_min) x_min = x_position;
    if (x_position > x_max) x_max = x_position;
    double x_scaled = scale_value(x_position, x_min, x_max);

    // printf("Joystick current: X = %d, Y = %d\n", x_position, y_position);
    // printf("Joystick scaled current: X = %f, Y = %f\n", x_scaled, y_scaled);

    struct JoystickData data = {
        .x = x_scaled,
        .y = y_scaled,
        .isPressed = false //Handled in button thread
    };

    return data;
}
