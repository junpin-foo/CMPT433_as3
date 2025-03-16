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

Period_statistics_t TerminalOutput_getAccelStats() {
    assert(isInitialized);
    return accelStats;
}

Period_statistics_t TerminalOutput_getAudioStats() {
    assert(isInitialized);
    return audioStats;
}

static void sleepForMs(long long delayInMs) { 
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000; 
    long long delayNs = delayInMs * NS_PER_MS;  
    int seconds = delayNs / NS_PER_SECOND;  
    int nanoseconds = delayNs % NS_PER_SECOND;  
    struct timespec reqDelay = {seconds, nanoseconds}; 
    nanosleep(&reqDelay, (struct timespec *) NULL); 
}

static void* TerminalOutputThread(void* args) {
    (void) args;
    assert(isInitialized);
    while (isRunning) {
        accelStats = Accelerometer_getSamplingTime();
        audioStats = BeatPlayer_getAudioTimeing();
        int beatMode = BeatPlayer_getBeatMode();
        int bpm = BeatPlayer_getBpm();
        int volume = BeatPlayer_getVolume();
        printf("M%d %dbpm vol:%d Audio[%f, %f] avg %f/%d Accel[%f, %f] avg %f/%d\n", beatMode, bpm, volume, 
            audioStats.minPeriodInMs, audioStats.maxPeriodInMs, audioStats.avgPeriodInMs, audioStats.numSamples,
            accelStats.minPeriodInMs, accelStats.maxPeriodInMs, accelStats.avgPeriodInMs, accelStats.numSamples);
        sleepForMs(1000);
    }
    return NULL;
}


void TerminalOutput_cleanup() {
    assert(isInitialized);
    isRunning = false;
    pthread_join(outputThread, NULL);
    isInitialized = false;
}