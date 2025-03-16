#include "hal/audioMixer.h"
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

#define DEFAULT_BPM 120
#define MIN_BPM 40
#define MAX_BPM 300

#define DEFAULT_VOLUME 80
#define MIN_VOLUME 0
#define MAX_VOLUME 100
#define BPM_PER_SPIN 5
#define MIN_COUNTER_ROTARY_ENCODER -16
#define MAX_COUNTER_ROTARY_ENCODER 36

static int volumn = DEFAULT_VOLUME;
static atomic_int bpm = DEFAULT_BPM;
static atomic_int beatMode = 1; // 1 = Rock, 2 = Custom, 0 = None
static atomic_int rotaryEncouderCounter = 0;
static bool isRunning = true;
static pthread_t beatThread;
static pthread_t bmpThread;
static wavedata_t hiHat;
static wavedata_t baseDrum;
static wavedata_t snare;
static bool isInitialized = false;

static void* beatThreadFunction(void* args);
static void* beatThreadDetectBPM(void* args);
static void BeatPlayer_playRockBeat(struct timespec halfBeatTime);
static void BeatPlayer_playCustomBeat(struct timespec halfBeatTime);
static void BeatPlayer_computeBPM();
// Initialize to play the rock beat
void BeatPlayer_init() {
    assert(!isInitialized);
    beatMode = 1;
    AudioMixer_init();
    AudioMixer_readWaveFileIntoMemory(HI_HAT_FILE, &hiHat);
    AudioMixer_readWaveFileIntoMemory(BASE_DRUM_FILE, &baseDrum);
    AudioMixer_readWaveFileIntoMemory(SNARE_FILE, &snare);
    isInitialized = true;
    RotaryEncoderStateMachine_init();
    BtnStateMachine_init();
    pthread_create(&beatThread, NULL, &beatThreadFunction, NULL);
    pthread_create(&bmpThread, NULL, &beatThreadDetectBPM, NULL);
}

static void* beatThreadFunction(void* args) {
    (void) args;
    assert(isInitialized);
    struct timespec halfBeatTime;
    while (isRunning) {
        int delayMs = (60 * 1000) / bpm / 2;                 // Calculate half-beat time in ms
        halfBeatTime.tv_sec  = delayMs / 1000;               // Convert ms to full seconds
        halfBeatTime.tv_nsec = (delayMs % 1000) * 1000000L;  // Convert remaining ms to ns
        beatMode = BtnStateMachine_getValue();
        if (beatMode == 1) { // Rock Beat
            BeatPlayer_playRockBeat(halfBeatTime);
        } else if (beatMode == 2) { // Custom Beat
            BeatPlayer_playCustomBeat(halfBeatTime);
        }
    }
    return NULL;
}

static void *beatThreadDetectBPM(void *args) {
    (void) args;
    assert(isInitialized);
    while (isRunning) {
        BeatPlayer_computeBPM();
    }
    return NULL;
}

void BeatPlayer_playHiHat() {
    assert(isInitialized);
    Period_markEvent(PERIOD_EVENT_SAMPLE_SOUND);
    AudioMixer_queueSound(&hiHat);
}

void BeatPlayer_playBaseDrum() {
    assert(isInitialized);
    Period_markEvent(PERIOD_EVENT_SAMPLE_SOUND);
    AudioMixer_queueSound(&baseDrum);
}

void BeatPlayer_playSnare() {
    assert(isInitialized);
    Period_markEvent(PERIOD_EVENT_SAMPLE_SOUND);
    AudioMixer_queueSound(&snare);
}

int BeatPlayer_getBpm() {
    assert(isInitialized);
    return bpm;
}

int BeatPlayer_getVolume() {
    assert(isInitialized);
    return AudioMixer_getVolume();
}

void BeatPlayer_setVolume(int newVolume) {
    assert(isInitialized);
    if (newVolume < MIN_VOLUME) {
        newVolume = MIN_VOLUME;
    } else if (newVolume > MAX_VOLUME) {
        newVolume = MAX_VOLUME;
    }
    volumn = newVolume;
    AudioMixer_setVolume(volumn);
}

void BeatPlayer_setBeatMode(int mode) {
    assert(isInitialized);
    BtnStateMachine_setValue(mode);
    beatMode = mode;
}

Period_statistics_t BeatPlayer_getAudioTimeing() {
    assert(isInitialized);
    Period_statistics_t stats;
    Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_SOUND, &stats);
    return stats;
}

int BeatPlayer_getBeatMode() {
    assert(isInitialized);
    return beatMode;
}

static void BeatPlayer_computeBPM() {
    assert(isInitialized);
    int new_rotaryEncouderCounter = RotaryEncoderStateMachine_getValue();
    if (new_rotaryEncouderCounter != rotaryEncouderCounter) {
        rotaryEncouderCounter = new_rotaryEncouderCounter;
        int new_bpm = DEFAULT_BPM + rotaryEncouderCounter * BPM_PER_SPIN;
        if (new_bpm < MIN_BPM) {
            bpm = MIN_BPM;
            RotaryEncoderStateMachine_setValue(MIN_COUNTER_ROTARY_ENCODER);
        } else if (new_bpm > MAX_BPM) {
            bpm = MAX_BPM;
            RotaryEncoderStateMachine_setValue(MAX_COUNTER_ROTARY_ENCODER);
        } else {
            bpm = new_bpm;
        }
    }
}

void BeatPlayer_setBPM(int newBpm) {
    assert(isInitialized);
    if (newBpm < MIN_BPM) {
        newBpm = MIN_BPM;
    } else if (newBpm > MAX_BPM) {
        newBpm = MAX_BPM;
    }
    bpm = newBpm;
    RotaryEncoderStateMachine_setValue((bpm - DEFAULT_BPM) / 5);
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
    isRunning = false;
    pthread_join(beatThread, NULL);
    pthread_join(bmpThread, NULL);
    AudioMixer_freeWaveFileData(&hiHat);
    AudioMixer_freeWaveFileData(&baseDrum);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_cleanup();
    BtnStateMachine_cleanup();
    RotaryEncoderStateMachine_cleanup();
    isInitialized = false;
}

