/* beatPlayer.h
 * 
 * This header file defines the necessary functions and structures to implement a 
 * beat player system that controls the playback of drum sounds (Hi-Hat, Bass Drum, Snare).
 * The `BeatPlayer_init()` function must be called before using any other functions 
 * in this module, as it initializes the necessary components (such as the audio mixer, 
 * file loading, and state machines) and starting a thread to repeately play the chosen beats.
 * 
 * The module uses sound files for different drum sounds. These files must be available 
 * at the specified locations (e.g., "beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav" 
 * for Bass Drum, "wbeatbox-wave-files100053__menegass__gui-drum-cc.wav" for Hi-Hat, and 
 * "beatbox-wave-files/100059__menegass__gui-drum-snare-soft.wav" for Snare).
 * 
 * Functions provided by this module allow for control over sound playback, BPM (beats 
 * per minute), volume, and the ability to switch between different beat modes.
 * 
 */

#ifndef BEAT_HELPER_H
#define BEAT_HELPER_H
#include "periodTimer.h"
#include <stdbool.h>

#define BASE_DRUM_FILE "beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav"
#define HI_HAT_FILE "beatbox-wave-files/100053__menegass__gui-drum-cc.wav"
#define SNARE_FILE "beatbox-wave-files/100059__menegass__gui-drum-snare-soft.wav"
#define NONE_MODE 0
#define ROCK_MODE 1
#define CUSTOM_MODE 2

// Structure to store a single step of a beat pattern.
// Each step can include up to three drum sounds: Hi-Hat, Bass Drum, and Snare.
// If a value is true, that instrument will be played during that step.
typedef struct {
    bool playHiHat;    // True if Hi-Hat should be played
    bool playBaseDrum; // True if Bass Drum should be played
    bool playSnare;    // True if Snare should be played
} Beat;

// Initialize/clean up the resource to play sound
void BeatPlayer_init();
void BeatPlayer_cleanup();

// Play corresponding sound
void BeatPlayer_playHiHat();
void BeatPlayer_playBaseDrum();
void BeatPlayer_playSnare();

// Set/Get the BPM of the playing sound
int BeatPlayer_getBpm();
void BeatPlayer_setBPM(int newBpm);

// Set/Get the volumn of plaaaying sound
int BeatPlayer_getVolume();
void BeatPlayer_setVolume(int newVolume);

// Set the playing mode of this module.
// Mode 0 -> No music pplay, Mode 1 -> play rock beat, Mode 2 -> play custom beat
int BeatPlayer_getBeatMode();
void BeatPlayer_setBeatMode(int mode);


#endif