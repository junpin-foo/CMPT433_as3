/* beat_box.c
*/

#include "hal/audioMixer.h"
#include <stdio.h>
#include <unistd.h>

#define SOURCE_FILE "wave-files/100051__menegass__gui-drum-bd-hard.wav"

int main(void)
{
	printf("Beginning play-back of %s\n", SOURCE_FILE);

	AudioMixer_init();

	wavedata_t sampleFile;

    while(1) {
        AudioMixer_setVolume(50);
		// Read source file data in to sampleFile pointer
        AudioMixer_readWaveFileIntoMemory(SOURCE_FILE, &sampleFile);
		// Queue the sound data into module
        AudioMixer_queueSound(&sampleFile);
        // AudioMixer_freeWaveFileData(&sampleFile);
        sleep(1);
    }

	// Cleanup, letting the music in buffer play out (drain), then close and free.
    AudioMixer_freeWaveFileData(&sampleFile);
    AudioMixer_cleanup();

	return 0;
}