#include "hal/audioMixer.h"
#include "beatPlayer.h"
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#define DEFAULT_BPM 120
#define MIN_BPM 40
#define MAX_BPM 300

#define DEFAULT_VOLUME 80
#define MIN_VOLUME 0
#define MAX_VOLUME 100

static int volumn = DEFAULT_VOLUME;
static int bpm = DEFAULT_BPM;
static int beatMode = 1; // 0 = None, 1 = Rock, 2 = Custom
static bool isRunning = true;
static pthread_t beatThread;
static pthread_mutex_t beatMutex = PTHREAD_MUTEX_INITIALIZER;
static wavedata_t hiHat;
static wavedata_t baseDrum;
static wavedata_t snare;
static bool isInitialized = false;

static void* beatThreadFunction(void* args);
static void BeatPlayer_playRockBeat(struct timespec halfBeatTime);
static void BeatPlayer_playCustomBeat(struct timespec halfBeatTime);

// Initialize to play the rock beat
void BeatPlayer_init() {
    assert(!isInitialized);
    beatMode = 1;
    AudioMixer_init();
    AudioMixer_readWaveFileIntoMemory(HI_HAT_FILE, &hiHat);
    AudioMixer_readWaveFileIntoMemory(BASE_DRUM_FILE, &baseDrum);
    AudioMixer_readWaveFileIntoMemory(SNARE_FILE, &snare);
    isInitialized = true;
    pthread_create(&beatThread, NULL, &beatThreadFunction, NULL);
}

static void* beatThreadFunction(void* args) {
    (void) args;
    assert(isInitialized);
    struct timespec halfBeatTime;
    while (isRunning) {
        pthread_mutex_lock(&beatMutex);
        int delayMs = (60 * 1000) / bpm / 2;                 // Calculate half-beat time in ms
        halfBeatTime.tv_sec  = delayMs / 1000;               // Convert ms to full seconds
        halfBeatTime.tv_nsec = (delayMs % 1000) * 1000000L;  // Convert remaining ms to ns
        if (beatMode == 1) { // Rock Beat
            BeatPlayer_playRockBeat(halfBeatTime);
        } else if (beatMode == 2) { // Custom Beat
            BeatPlayer_playCustomBeat(halfBeatTime);
        }
        pthread_mutex_unlock(&beatMutex);
    }
    return NULL;
}

void BeatPlayer_playHiHat() {
    assert(isInitialized);
    AudioMixer_queueSound(&hiHat);
}

void BeatPlayer_playBaseDrum() {
    assert(isInitialized);
    AudioMixer_queueSound(&baseDrum);
}

void BeatPlayer_playSnare() {
    assert(isInitialized);
    AudioMixer_queueSound(&snare);
}

int BeatPlayer_getBpm() {
    assert(isInitialized);
    return bpm;
}

void BeatPlayer_setBpm(int newBpm) {
    assert(isInitialized);
    pthread_mutex_lock(&beatMutex);
    if (newBpm < MIN_BPM) {
        bpm = MIN_BPM;
    } else if (newBpm > MAX_BPM) {
        bpm = MAX_BPM;
    } else {
        bpm = newBpm;
    }
    pthread_mutex_unlock(&beatMutex);
}

int BeatPlayer_getVolume() {
    assert(isInitialized);
    return AudioMixer_getVolume();
}

void BeatPlayer_setVolume(int newVolume) {
    assert(isInitialized);
    pthread_mutex_lock(&beatMutex);
    if (newVolume < MIN_VOLUME) {
        newVolume = MIN_VOLUME;
    } else if (newVolume > MAX_VOLUME) {
        newVolume = MAX_VOLUME;
    }
    volumn = newVolume;
    AudioMixer_setVolume(volumn);
    pthread_mutex_unlock(&beatMutex);
}

static void BeatPlayer_playRockBeat(struct timespec halfBeatTime) {
    assert(isInitialized);
    // 1
    BeatPlayer_playHiHat(); 
    BeatPlayer_playBaseDrum(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
    // 1.5
    BeatPlayer_playHiHat(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
    // 2
    BeatPlayer_playHiHat(); 
    BeatPlayer_playSnare(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
    // 2.5
    BeatPlayer_playHiHat(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
    // 3
    BeatPlayer_playHiHat();
    BeatPlayer_playBaseDrum(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
    // 3.5
    BeatPlayer_playHiHat(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
    // 4
    BeatPlayer_playHiHat(); 
    BeatPlayer_playSnare(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
    // 4.5
    BeatPlayer_playHiHat(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
}

// I might change it later to make it more distinct with the Rock Beat
static void BeatPlayer_playCustomBeat(struct timespec halfBeatTime) {
    assert(isInitialized);
    BeatPlayer_playBaseDrum(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time

    BeatPlayer_playSnare(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time

    BeatPlayer_playHiHat(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time

    BeatPlayer_playHiHat(); 
    BeatPlayer_playBaseDrum(); 
    nanosleep(&halfBeatTime, NULL); // Sleep for the calculated time
}

void BeatPlayer_cleanup() {
    assert(isInitialized);
    pthread_join(beatThread, NULL);
    AudioMixer_freeWaveFileData(&hiHat);
    AudioMixer_freeWaveFileData(&baseDrum);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_cleanup();
    isInitialized = false;
}

