/* terminalOutput.c
 * 
 * This file contains the implementation of the terminal output thread. 
 * The thread prints the current beat mode, bpm, volume, audio statistics, and accelerometer statistics to the terminal every second.
 *
 */


#include "beatPlayer.h"
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <periodTimer.h>
#include <stdatomic.h>
#include <hal/rotary_encoder_statemachine.h>
#include <hal/rotary_btn_statemachine.h>
#include <hal/accelerometer.h>
#include <terminalOutput.h>
#include "beatPlayer.h"
#include "hal/audioMixer.h"
#include "sleep_timer_helper.h"
#include "updateLcd.h"

#define ONE_SECOND_IN_MS 1000
static bool isInitialized = false;
static pthread_t outputThread;
static bool isRunning = true;
static Period_statistics_t accelStats;
static Period_statistics_t audioStats;

static void* TerminalOutputThread(void* args);
void TerminalOutput_init() {
    assert(!isInitialized);
    isInitialized = true;
    pthread_create(&outputThread, NULL, &TerminalOutputThread, NULL);
}

void TerminalOutput_cleanup() {
    assert(isInitialized);
    isRunning = false;
    pthread_join(outputThread, NULL);
    isInitialized = false;
}

Period_statistics_t TerminalOutput_getAccelStats() {
    assert(isInitialized);
    return accelStats;
}

Period_statistics_t TerminalOutput_getAudioStats() {
    assert(isInitialized);
    return audioStats;
}

static void* TerminalOutputThread(void* args) {
    (void) args;
    assert(isInitialized);
    while (isRunning) {
        accelStats = Accelerometer_getSamplingTime();
        audioStats = AudioMixer_getAudioStat();
        int beatMode = BeatPlayer_getBeatMode();
        int bpm = BeatPlayer_getBpm();
        int volume = BeatPlayer_getVolume();
        printf("M%d %dbpm vol:%d  Audio[%.3f, %.3f] avg %.3f/%d  Accel[%.3f, %.3f] avg %.3f/%d\n", beatMode, bpm, volume, 
            audioStats.minPeriodInMs, audioStats.maxPeriodInMs, audioStats.avgPeriodInMs, audioStats.numSamples,
            accelStats.minPeriodInMs, accelStats.maxPeriodInMs, accelStats.avgPeriodInMs, accelStats.numSamples);
        sleepForMs(ONE_SECOND_IN_MS);
    }
    return NULL;
}
