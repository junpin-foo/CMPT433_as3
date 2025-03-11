#ifndef BEAT_HELPER_H
#define BEAT_HELPER_H

#define BASE_DRUM_FILE "wave-files/100051__menegass__gui-drum-bd-hard.wav"
#define HI_HAT_FILE "wave-files/100053__menegass__gui-drum-cc.wav"
#define SNARE_FILE "wave-files/100059__menegass__gui-drum-snare-soft.wav"

void beat_helper_init();
void playHiHat();
void playBaseDrum();
void playSnare();
void beat_helper_cleanup();
#endif