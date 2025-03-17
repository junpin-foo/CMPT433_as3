/* rotary_btn_statemachine.h 
* Header file for the rotary button state machine implementation.
* This module is responsible for monitoring and processing rotary button
* signals using a state machine approach.
*/
#ifndef _BTN_STATEMACHINE_H_
#define _BTN_STATEMACHINE_H_

#include <stdbool.h>

// Start thread to monitor the rotary button
void BtnStateMachine_init(void);

// Clean up the rotary button state machine
void BtnStateMachine_cleanup(void);

// Get the current value of the rotary button
void BtnStateMachine_setValue(int value);

// Set the value of the rotary button (mainly for resetting purposes)
int BtnStateMachine_getValue();

#endif