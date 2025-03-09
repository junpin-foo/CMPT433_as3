/* beat_box.c
*/

#include <wave_player.h>
// #include "hal/audioMixer.h"

#define SOURCE_FILE "wave-files/100051__menegass__gui-drum-bd-hard.wav"

int main(void)
{
	printf("Beginning play-back of %s\n", SOURCE_FILE);

	// Configure Output Device
	snd_pcm_t *handle = Audio_openDevice();

	// Load wave file we want to play:
	wavedata_t sampleFile;
	Audio_readWaveFileIntoMemory(SOURCE_FILE, &sampleFile);

	// Play Audio
	// Audio_playFile(handle, &sampleFile);
	// Audio_playFile(handle, &sampleFile);
	// Audio_playFile(handle, &sampleFile);
    while(1) {
        Audio_playFile(handle, &sampleFile);
        sleep(1);
    }

	// Cleanup, letting the music in buffer play out (drain), then close and free.
	snd_pcm_drain(handle);
	snd_pcm_hw_free(handle);
	snd_pcm_close(handle);
	free(sampleFile.pData);

	printf("Done!\n");
	return 0;
}