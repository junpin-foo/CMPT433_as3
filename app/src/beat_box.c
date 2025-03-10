/* beat_box.c
*/

#include "hal/audioMixer.h"
#include <stdio.h>
#include <unistd.h>

#define SOURCE_FILE "wave-files/100055__menegass__gui-drum-co.wav"
#define SOURCE_FILE2 "wave-files/100066__menegass__gui-drum-tom-mid-hard.wav"
int main(void)
{
	printf("Beginning play-back of %s\n", SOURCE_FILE);
    printf("Beginning play-back of %s\n", SOURCE_FILE2);
	AudioMixer_init();

	wavedata_t sampleFile;
    wavedata_t sampleFile2;
    AudioMixer_setVolume(70);
    while(1) {
		// Read source file data in to sampleFile pointer
        AudioMixer_readWaveFileIntoMemory(SOURCE_FILE, &sampleFile);
        AudioMixer_readWaveFileIntoMemory(SOURCE_FILE2, &sampleFile2);

		// Queue the sound data into module. Comment out the queued(sample1) or queued(sample2) to test whether mix work 
        AudioMixer_queueSound(&sampleFile);
        AudioMixer_queueSound(&sampleFile2);
        
        // AudioMixer_freeWaveFileData(&sampleFile);
        // AudioMixer_freeWaveFileData(&sampleFile2);
        sleep(1);
    }

	// Cleanup, letting the music in buffer play out (drain), then close and free.
    // AudioMixer_freeWaveFileData(&sampleFile);
    AudioMixer_cleanup();

	return 0;
}