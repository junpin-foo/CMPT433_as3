
#include "hal/rotary_btn_statemachine.h"
#include "hal/gpio.h"
#include "sleep_timer_helper.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

#define GPIO_CHIP          GPIO_CHIP_0
#define GPIO_LINE_NUMBER   10

static pthread_t stateMachineThread;

static bool isInitialized = false;

struct GpioLine* s_lineBtn = NULL;
static atomic_int counter = 1;
static bool stateMachineRunning = false;

//DEBOUNCE
#define DEBOUNCE_TIME_MS 100
static struct timespec last_btn_time;

//PROTOTYPE
static void* BtnStateMachine_doState(void* arg);

/*
    Define the Statemachine Data Structures
*/
struct stateEvent {
    struct state* pNextState;
    void (*action)();
};
struct state {
    struct stateEvent rising;
    struct stateEvent falling;
};


static void on_release(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    if(time_diff_ms(&last_btn_time, &now) > DEBOUNCE_TIME_MS) {
        int new_counter = (atomic_load(&counter) + 1) % 3;
        atomic_store(&counter, new_counter);
        last_btn_time = now;
    }
}

static struct state states[] = {
    { // Not pressed
        .rising = {&states[0], NULL},
        .falling = {&states[1], NULL},
    },

    { // Pressed
        .rising = {&states[0], on_release},
        .falling = {&states[1], NULL},
    },
};

struct state* pCurrentState = &states[0];

void BtnStateMachine_init()
{
    assert(!isInitialized);
    stateMachineRunning = true;
    isInitialized = true;
    s_lineBtn = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER);
    pthread_create(&stateMachineThread, NULL, &BtnStateMachine_doState, NULL);
}
void BtnStateMachine_cleanup()
{
    assert(isInitialized);
    stateMachineRunning = false;
    pthread_join(stateMachineThread, NULL);
    Gpio_close(s_lineBtn);
    isInitialized = false;
}

int BtnStateMachine_getValue()
{
    return counter;
}

void BtnStateMachine_setValue(int value)
{
    counter = value;
}

static void* BtnStateMachine_doState(void* arg)
{
    (void)arg; // Suppress unused parameter warning
    assert(isInitialized);

    // printf("\n\nWaiting for an event...\n");
    while (stateMachineRunning) {
        struct gpiod_line_bulk bulkEvents;
        int numEvents = Gpio_waitFor1LineChange(s_lineBtn, &bulkEvents);
        if (numEvents == -1) {
            break;  // Exit the loop on failure
        }

        // Iterate over the event
        for (int i = 0; i < numEvents; i++)
        {
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

            // Get the number of this line
            unsigned int this_line_number = gpiod_line_offset(line_handle);

            // Get the line event
            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle,&event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }


            // Run the state machine
            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

            // Can check with line it is, if you have more than one...
            bool isBtn = this_line_number == GPIO_LINE_NUMBER;
            assert (isBtn);

            struct stateEvent* pStateEvent = NULL;
            if (isRising) {
                pStateEvent = &pCurrentState->rising;
            } else {
                pStateEvent = &pCurrentState->falling;
            } 

            // Do the action
            if (pStateEvent->action != NULL) {
                pStateEvent->action();
            }
            pCurrentState = pStateEvent->pNextState;

            // DEBUG INFO ABOUT STATEMACHINE
            #if 0
            int newState = (pCurrentState - &states[0]);
            double time = event.ts.tv_sec + event.ts.tv_nsec / 1000000000.0;
            printf("State machine Debug: i=%d/%d  line num/dir = %d %8s -> new state %d     [%f]\n", 
                i, 
                numEvents,
                this_line_number, 
                isRising ? "RISING": "falling", 
                newState,
                time);
            #endif
        }
    }
    return NULL;
}

