/* beatPlayer.c
 * 
 * This file implements the functions defined in beatPlayer.h. 
 * It provides the necessary functions to control the playback of drum sounds (Hi-Hat, Bass Drum, Snare
 * and to switch between different beat modes.
 * 
 */

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
#include <sleep_timer_helper.h>
#include "hal/joystick.h"

#define DEFAULT_BPM 120
#define MIN_BPM 40
#define MAX_BPM 300
#define DEFAULT_DELAY_MS 100

#define DEFAULT_VOLUME 80
#define MIN_VOLUME 0
#define MAX_VOLUME 100
#define BPM_PER_SPIN 5
#define MIN_COUNTER_ROTARY_ENCODER -16
#define MAX_COUNTER_ROTARY_ENCODER 36
#define HALF_BEAT_RATIO ((60*1000)/2)
#define ROCK_BEAT_STRUCT_SIZE 8
#define CUSTOM_BEAT_STRUCT_SIZE 4



static atomic_int volume = DEFAULT_VOLUME;
static atomic_int bpm = DEFAULT_BPM;
static atomic_int beatMode = 1; // 1 = Rock, 2 = Custom, 0 = None
static bool isRunning = true;
static pthread_t beatThread;
static pthread_t bmpThread;
static pthread_t volumeThread;
static wavedata_t hiHat;
static wavedata_t baseDrum;
static wavedata_t snare;
static bool isInitialized = false;
static atomic_int halfBeatTimeMs = 0;

static Beat rockBeat[ROCK_BEAT_STRUCT_SIZE] = {
    {true,  true,  false}, // 1   - Hi-Hat & Bass Drum
    {true,  false, false}, // 1.5 - Hi-Hat
    {true,  false, true }, // 2   - Hi-Hat & Snare
    {true,  false, false}, // 2.5 - Hi-Hat
    {true,  true,  false}, // 3   - Hi-Hat & Bass Drum
    {true,  false, false}, // 3.5 - Hi-Hat
    {true,  false, true }, // 4   - Hi-Hat & Snare
    {true,  false, false}  // 4.5 - Hi-Hat
};

static Beat customBeat[CUSTOM_BEAT_STRUCT_SIZE] = {
    {false, true,  false}, // 1   - Bass Drum
    {false, false, true }, // 1.5 - Snare
    {true,  false, false}, // 2   - Hi-Hat
    {true,  true,  false}  // 2.5 - Hi-Hat & Bass Drum
};

static void* beatThreadFunction(void* args);
static void* beatThreadDetectBPM(void* args);
static void* beatThreadSetVolume(void* args);
static void BeatPlayer_playRockBeat();
static void BeatPlayer_playCustomBeat();
static void BeatPlayer_detectRotarySpin();

void BeatPlayer_init() {
    assert(!isInitialized);
    beatMode = 1;
    halfBeatTimeMs = (HALF_BEAT_RATIO / bpm); 
    AudioMixer_init();
    RotaryEncoderStateMachine_init();
    BtnStateMachine_init();
    isInitialized = true;
    AudioMixer_readWaveFileIntoMemory(HI_HAT_FILE, &hiHat);
    AudioMixer_readWaveFileIntoMemory(BASE_DRUM_FILE, &baseDrum);
    AudioMixer_readWaveFileIntoMemory(SNARE_FILE, &snare);
    pthread_create(&beatThread, NULL, &beatThreadFunction, NULL);
    pthread_create(&bmpThread, NULL, &beatThreadDetectBPM, NULL);
    pthread_create(&volumeThread, NULL, &beatThreadSetVolume, NULL);
}

void BeatPlayer_cleanup() {
    assert(isInitialized);
    isRunning = false;
    pthread_join(beatThread, NULL);
    pthread_join(bmpThread, NULL);
    pthread_join(volumeThread, NULL);
    AudioMixer_freeWaveFileData(&hiHat);
    AudioMixer_freeWaveFileData(&baseDrum);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_cleanup();
    BtnStateMachine_cleanup();
    RotaryEncoderStateMachine_cleanup();
    isInitialized = false;
}

static void* beatThreadFunction(void* args) {
    (void) args;
    assert(isInitialized);
    while (isRunning) {
        beatMode = BtnStateMachine_getValue();
        if (beatMode == ROCK_MODE) { // Rock Beat
            BeatPlayer_playRockBeat();
        } else if (beatMode == CUSTOM_MODE) { // Custom Beat
            BeatPlayer_playCustomBeat();
        } else {
            sleepForMs(DEFAULT_DELAY_MS);
        }
    }
    return NULL;
}

static void *beatThreadDetectBPM(void *args) {
    (void) args;
    assert(isInitialized);
    while (isRunning) {
        BeatPlayer_detectRotarySpin();
        sleepForMs(DEFAULT_DELAY_MS);
    }
    return NULL;
}


static void* beatThreadSetVolume(void* args) {
    (void) args;
    assert(isInitialized);
    while (isRunning) {
        JoystickDirection data = getJoystickDirection();
        if (data == JOYSTICK_UP) {
            int new_volume = atomic_load(&volume) + 5;
            if (new_volume <= 100) atomic_store(&volume, new_volume);
            BeatPlayer_setVolume(new_volume);
        } else if (data == JOYSTICK_DOWN) {
            int new_volume = atomic_load(&volume) - 5;
            if (new_volume >= 0) atomic_store(&volume, new_volume);
            BeatPlayer_setVolume(new_volume);
        }
        sleepForMs(DEFAULT_DELAY_MS);
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
    volume = newVolume;
    AudioMixer_setVolume(volume);
}

void BeatPlayer_setBeatMode(int mode) {
    assert(isInitialized);
    BtnStateMachine_setValue(mode);
    beatMode = mode;
}

int BeatPlayer_getBeatMode() {
    assert(isInitialized);
    return beatMode;
}

static void BeatPlayer_detectRotarySpin() {
    assert(isInitialized);
    int rotaryEncouderCounter = RotaryEncoderStateMachine_getValue();
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
    halfBeatTimeMs = (HALF_BEAT_RATIO / bpm);
}

void BeatPlayer_setBPM(int newBpm) {
    assert(isInitialized);
    if (newBpm < MIN_BPM) {
        newBpm = MIN_BPM;
    } else if (newBpm > MAX_BPM) {
        newBpm = MAX_BPM;
    }
    bpm = newBpm;
    halfBeatTimeMs = (HALF_BEAT_RATIO / bpm);
    RotaryEncoderStateMachine_setValue((bpm - DEFAULT_BPM) / BPM_PER_SPIN);
}

static void BeatPlayer_playRockBeat() {
    assert(isInitialized);
    for (int i = 0; i < ROCK_BEAT_STRUCT_SIZE; i++) {
        // Play Hi-Hat if specified
        if (rockBeat[i].playHiHat) {
            BeatPlayer_playHiHat();
        }

        // Play Bass Drum if specified
        if (rockBeat[i].playBaseDrum) {
            BeatPlayer_playBaseDrum();
        }

        // Play Snare if specified
        if (rockBeat[i].playSnare) {
            BeatPlayer_playSnare();
        }

        // Sleep for the calculated half-beat time
        sleepForMs(halfBeatTimeMs);
    }
}

static void BeatPlayer_playCustomBeat() {
    assert(isInitialized);
    for (int i = 0; i < CUSTOM_BEAT_STRUCT_SIZE; i++) {
        // Play Hi-Hat if specified
        if (customBeat[i].playHiHat) {
            BeatPlayer_playHiHat();
        }

        // Play Bass Drum if specified
        if (customBeat[i].playBaseDrum) {
            BeatPlayer_playBaseDrum();
        }

        // Play Snare if specified
        if (customBeat[i].playSnare) {
            BeatPlayer_playSnare();
        }

        // Sleep for the calculated half-beat time
        sleepForMs(halfBeatTimeMs);
    }
}