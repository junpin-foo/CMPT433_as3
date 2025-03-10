#include "hal/audioMixer.h"
#include "beat_helper.h"
#include <stdio.h>
#include <unistd.h>

wavedata_t hiHat;
wavedata_t baseDrum;
wavedata_t snare;

void heat_helper_init() {
    AudioMixer_init();
}

void playHiHat() {
    AudioMixer_readWaveFileIntoMemory(HI_HAT_FILE, &hiHat);
    AudioMixer_queueSound(&hiHat);
}

void playBaseDrum() {
    AudioMixer_readWaveFileIntoMemory(BASE_DRUM_FILE, &baseDrum);
    AudioMixer_queueSound(&baseDrum);
}

void playSnare() {
    AudioMixer_readWaveFileIntoMemory(SNARE_FILE, &snare);
    AudioMixer_queueSound(&snare);
}

void beat_helper_cleanup() {
    AudioMixer_freeWaveFileData(&hiHat);
    AudioMixer_freeWaveFileData(&baseDrum);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_cleanup();
}

