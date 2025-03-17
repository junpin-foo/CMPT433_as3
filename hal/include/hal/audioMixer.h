/* audioMixer.h
* This module provides a simple interface for playing back wave files, adding them to a queue, and setting the volume.
* It uses the ALSA library to play back the wave files in real time, allowing multiple wave files to be mixed together and played without jitter.
*/

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H
#include "periodTimer.h"

typedef struct {
	int numSamples;
	short *pData;
} wavedata_t;

#define AUDIOMIXER_MAX_VOLUME 100

// init() must be called before any other functions,
// cleanup() must be called last to stop playback threads and free memory.
void AudioMixer_init(void);
void AudioMixer_cleanup(void);

// Read the contents of a wave file into the pSound structure. Note that
// the pData pointer in this structure will be dynamically allocated in
// readWaveFileIntoMemory(), and is freed by calling freeWaveFileData().
void AudioMixer_readWaveFileIntoMemory(char *fileName, wavedata_t *pSound);

// Free the memory allocated for the wave file data.
void AudioMixer_freeWaveFileData(wavedata_t *pSound);

// Queue up another sound bite to play as soon as possible.
void AudioMixer_queueSound(wavedata_t *pSound);

// Get/set the volume.
// setVolume() function posted by StackOverflow user "trenki" at:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
int  AudioMixer_getVolume();

// Set the volume to a value between 0 and 100.
void AudioMixer_setVolume(int newVolume);

// Get the current audio statistics.
Period_statistics_t AudioMixer_getAudioStat();

#endif
