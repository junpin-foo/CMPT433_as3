/* reaction_timer.h
    Header file for the Reaction Timer game, which measures a user's reaction time
    based on joystick input and provides feedback using LEDs.
    This file defines constants, types, and function prototypes for handling 
    joystick input, LED control, and game logic.
 */

#ifndef REACTION_TIMER_H
#define REACTION_TIMER_H

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "hal/led.h"
#include "hal/joystick.h"

#define GREEN_LED &leds[0]
#define RED_LED &leds[1]
#define FLASH_DURATION_MS 250
#define MAX_RESPONSE_TIME_MS 5000

typedef enum {
    JOYSTICK_CENTER,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT
} JoystickDirection;

#endif
