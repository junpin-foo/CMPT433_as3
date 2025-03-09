/* wave_player.h 
 * 
 */

#ifndef _WAVEPLAYER_H_
#define _WAVEPLAYER_H_

#include <stdint.h>
#include <alsa/asoundlib.h>

// Store data of a single wave file read into memory.
// Space is dynamically allocated; must be freed correctly!
typedef struct {
	int numSamples;
	short *pData;
} wavedata_t;

snd_pcm_t *Audio_openDevice();
void Audio_readWaveFileIntoMemory(char *fileName, wavedata_t *pWaveStruct);
void Audio_playFile(snd_pcm_t *handle, wavedata_t *pWaveData);

#endif