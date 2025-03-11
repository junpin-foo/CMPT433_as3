#include "hal/audioMixer.h"
#include "beat_helper.h"
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#define DEFAULT_BPM 120
#define MIN_BPM 40
#define MAX_BPM 300

static int bpm = DEFAULT_BPM;
static int beatMode = 1; // 0 = None, 1 = Rock, 2 = Custom
static bool isRunning = true;
static pthread_t beatThread;

static wavedata_t hiHat;
static wavedata_t baseDrum;
static wavedata_t snare;
static bool isInitialized = false;

static void* beatThreadFunction(void* args);

void beat_helper_init() {
    assert(!isInitialized);
    beatMode = 1;
    AudioMixer_init();
    AudioMixer_readWaveFileIntoMemory(HI_HAT_FILE, &hiHat);
    AudioMixer_readWaveFileIntoMemory(BASE_DRUM_FILE, &baseDrum);
    AudioMixer_readWaveFileIntoMemory(SNARE_FILE, &snare);
    isInitialized = true;
    pthread_create(&beatThread, NULL, &beatThreadFunction, NULL);
}

void playHiHat() {
    assert(isInitialized);
    AudioMixer_queueSound(&hiHat);
}

void playBaseDrum() {
    assert(isInitialized);
    AudioMixer_queueSound(&baseDrum);
}

void playSnare() {
    assert(isInitialized);
    AudioMixer_queueSound(&snare);
}


static void* beatThreadFunction(void* args) {
    (void) args;
    assert(isInitialized);
    struct timespec halfBeatTime;
    while (isRunning) {
        int delayMs = (60 * 1000) / bpm / 2;                 // Calculate half-beat time in ms
        halfBeatTime.tv_sec  = delayMs / 1000;               // Convert ms to full seconds
        halfBeatTime.tv_nsec = (delayMs % 1000) * 1000000L;  // Convert remaining ms to ns
        if (beatMode == 1) { // Rock Beat
            // 1
            playHiHat(); 
            playBaseDrum(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
            // 1.5
            playHiHat(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
            // 2
            playHiHat(); 
            playSnare(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
            // 2.5
            playHiHat(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
            // 3
            playHiHat();
            playBaseDrum(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
            // 3.5
            playHiHat(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
            // 4
            playHiHat(); 
            playSnare(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
            // 4.5
            playHiHat(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
        }
        else if (beatMode == 2) { // Custom Beat
            playBaseDrum(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time

            playSnare(); 

            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
            playHiHat(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time

            playHiHat(); 
            playBaseDrum(); 
            nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
        }
    }
    return NULL;
}

void beat_helper_cleanup() {
    assert(isInitialized);
    pthread_join(beatThread, NULL);
    AudioMixer_freeWaveFileData(&hiHat);
    AudioMixer_freeWaveFileData(&baseDrum);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_cleanup();
    isInitialized = false;
}

